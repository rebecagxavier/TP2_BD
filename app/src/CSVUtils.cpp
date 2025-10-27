#include "../include/CSVUtils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <cstdint>

using namespace std;

// --- Limpa caracteres indesejados, como <i>, &aacute;, etc. ---
string limparCampo(string campo) {
    if (campo.size() >= 2 && campo.front() == '"' && campo.back() == '"')
        campo = campo.substr(1, campo.size() - 2);

    if (campo == "NULL" || campo == "null")
        return "";

    const vector<pair<string, string>> htmlSubs = {
        {"&iacute;", "í"}, {"&aacute;", "á"}, {"&eacute;", "é"},
        {"&oacute;", "ó"}, {"&uacute;", "ú"}, {"&ccedil;", "ç"},
        {"<i>", ""}, {"</i>", ""}, {"<b>", ""}, {"</b>", ""},
        {"Ã¡", "á"}, {"Ã©", "é"}, {"Ã­", "í"}, {"Ã³", "ó"}, {"Ãº", "ú"},
        {"Ã£", "ã"}, {"Ãµ", "õ"}, {"Ã¢", "â"}, {"Ãª", "ê"}, {"Ã´", "ô"},
        {"Ã§", "ç"}, {"Ã‰", "É"}, {"Ã", "Ç"},
        {"ï¬", "fi"}, {"ï¬", "f"}, {"â", "⋆"}, {"â", "—"}, {"â", "–"}
    };

    for (auto &p : htmlSubs) {
        size_t pos;
        while ((pos = campo.find(p.first)) != string::npos)
            campo.replace(pos, p.first.length(), p.second);
    }

    // tenta normalizar o campo para UTF-8 (corrige CP1252, mojibake comum, etc.)
    campo = normalizeToUtf8(campo);

    campo.erase(remove(campo.begin(), campo.end(), '\r'), campo.end());
    campo.erase(remove(campo.begin(), campo.end(), '\n'), campo.end());
    return campo;
}

// ----------------- Helpers para correção/normalização de encoding -----------------

static bool isValidUTF8(const std::string &s) {
    const unsigned char *bytes = (const unsigned char *)s.c_str();
    size_t len = s.size();
    size_t i = 0;
    while (i < len) {
        unsigned char c = bytes[i];
        if (c <= 0x7F) { i += 1; continue; }
        else if ((c >> 5) == 0x6) { // 110xxxxx 10xxxxxx
            if (i + 1 >= len) return false;
            if ((bytes[i+1] >> 6) != 0x2) return false;
            i += 2;
        } else if ((c >> 4) == 0xE) { // 1110xxxx 10xxxxxx 10xxxxxx
            if (i + 2 >= len) return false;
            if ((bytes[i+1] >> 6) != 0x2 || (bytes[i+2] >> 6) != 0x2) return false;
            i += 3;
        } else if ((c >> 3) == 0x1E) { // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            if (i + 3 >= len) return false;
            if ((bytes[i+1] >> 6) != 0x2 || (bytes[i+2] >> 6) != 0x2 || (bytes[i+3] >> 6) != 0x2) return false;
            i += 4;
        } else return false;
    }
    return true;
}

// conta quantos codepoints > 127 (heurística para avaliar se texto tem acentuação)
static size_t countNonAscii(const std::string &s) {
    size_t cnt = 0;
    const unsigned char *bytes = (const unsigned char *)s.c_str();
    size_t len = s.size();
    size_t i = 0;
    while (i < len) {
        unsigned char c = bytes[i];
        if (c <= 0x7F) { i += 1; }
        else if ((c >> 5) == 0x6) { cnt++; i += 2; }
        else if ((c >> 4) == 0xE) { cnt++; i += 3; }
        else if ((c >> 3) == 0x1E) { cnt++; i += 4; }
        else { i += 1; }
    }
    return cnt;
}

static void appendUtf8ForCodepoint(std::string &out, uint32_t cp) {
    if (cp <= 0x7F) out.push_back((char)cp);
    else if (cp <= 0x7FF) {
        out.push_back((char)(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back((char)(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back((char)(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back((char)(0x80 | (cp & 0x3F)));
    } else {
        out.push_back((char)(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back((char)(0x80 | (cp & 0x3F)));
    }
}

// Converte bytes no encoding Windows-1252 para UTF-8 (fallback)
static std::string cp1252_to_utf8(const std::string &s) {
    static const uint32_t cp1252_map[32] = {
        0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
        0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178
    };

    std::string out;
    out.reserve(s.size() * 2);
    for (unsigned char c : s) {
        if (c < 0x80) out.push_back((char)c);
        else if (c >= 0x80 && c <= 0x9F) {
            uint32_t cp = cp1252_map[c - 0x80];
            if (cp == 0xFFFD) { // unknown — map conservatively to replacement
                appendUtf8ForCodepoint(out, 0xFFFD);
            } else appendUtf8ForCodepoint(out, cp);
        } else {
            // 0xA0..0xFF map to U+00A0 .. U+00FF
            appendUtf8ForCodepoint(out, 0x00FF & c);
        }
    }
    return out;
}

// Decodifica string UTF-8 para vetor de codepoints; retorna false se sequência inválida
static bool utf8_to_codepoints(const std::string &s, std::vector<uint32_t> &out) {
    out.clear();
    const unsigned char *bytes = (const unsigned char *)s.c_str();
    size_t len = s.size();
    size_t i = 0;
    while (i < len) {
        unsigned char c = bytes[i];
        if (c <= 0x7F) { out.push_back(c); i += 1; }
        else if ((c >> 5) == 0x6) {
            if (i + 1 >= len) return false;
            uint32_t cp = ((c & 0x1F) << 6) | (bytes[i+1] & 0x3F);
            out.push_back(cp); i += 2;
        } else if ((c >> 4) == 0xE) {
            if (i + 2 >= len) return false;
            uint32_t cp = ((c & 0x0F) << 12) | ((bytes[i+1] & 0x3F) << 6) | (bytes[i+2] & 0x3F);
            out.push_back(cp); i += 3;
        } else if ((c >> 3) == 0x1E) {
            if (i + 3 >= len) return false;
            uint32_t cp = ((c & 0x07) << 18) | ((bytes[i+1] & 0x3F) << 12) | ((bytes[i+2] & 0x3F) << 6) | (bytes[i+3] & 0x3F);
            out.push_back(cp); i += 4;
        } else return false;
    }
    return true;
}

// Se detectar padrões de 'mojibake' tenta reconstituir: interpreta os codepoints (<=255)
// como bytes originais e decodifica como UTF-8; se parecer melhor, retorna reconstruído.
static std::string try_fix_mojibake(const std::string &s) {
    // Heurística rápida: se contém 'Ã' ou 'Â' ou 'â' pode ser mojibake
    if (s.find("Ã") == std::string::npos && s.find("Â") == std::string::npos && s.find("â") == std::string::npos)
        return s; // nada a fazer

    std::vector<uint32_t> cps;
    if (!utf8_to_codepoints(s, cps)) {
        // inválido UTF-8 → tenta converter CP1252→UTF8
        return cp1252_to_utf8(s);
    }

    // se todos os codepoints são <=255, podemos tentar reconstruir bytes originais
    bool all_le_255 = true;
    for (uint32_t cp : cps) if (cp > 255) { all_le_255 = false; break; }

    if (!all_le_255) return s;

    // monta bytes a partir dos codepoints baixos e tenta decodificar como UTF-8
    std::string bytes; bytes.reserve(cps.size());
    for (uint32_t cp : cps) bytes.push_back(static_cast<char>(cp & 0xFF));

    if (isValidUTF8(bytes)) {
        // heurística: se o candidato tem mais caracteres não-ascii que o original, aceita
        if (countNonAscii(bytes) > countNonAscii(s)) return bytes;
    }

    // fallback: tenta CP1252->UTF8
    return cp1252_to_utf8(s);
}

// Normaliza um campo para UTF-8: tenta múltiplas heurísticas
static std::string normalizeToUtf8(const std::string &inp) {
    if (inp.empty()) return inp;
    if (isValidUTF8(inp)) {
        // mesmo que seja válido, pode ser mojibake (ex: contém 'Ã') — tenta corrigi-lo
        std::string maybe = try_fix_mojibake(inp);
        return maybe;
    } else {
        // inválido UTF-8: tenta CP1252->UTF8
        return cp1252_to_utf8(inp);
    }
}


// --- Divide uma linha completa em campos ---
vector<string> dividirCampos(string linha) {
    vector<string> campos;
    string campoAtual;
    bool dentroDeAspas = false;

    for (size_t i = 0; i < linha.size(); ++i) {
        char c = linha[i];

        if (c == '"') {
            dentroDeAspas = !dentroDeAspas; // alterna estado
        } else if (c == ';' && !dentroDeAspas) {
            // fim de campo — adiciona o atual
            campos.push_back(limparCampo(campoAtual));
            campoAtual.clear();
        } else {
            campoAtual += c;
        }
    }

    // adiciona o último campo, se existir
    if (!campoAtual.empty())
        campos.push_back(limparCampo(campoAtual));

    return campos;
}


// --- Conta o número de aspas em uma string ---
int contarAspas(const string &linha) {
    return count(linha.begin(), linha.end(), '"');
}

// --- Lê CSV tratando quebras de linha dentro de campos entre aspas ---
vector<Artigo> lerCSV(const string &nomeArquivo) {
    vector<Artigo> artigos;
    ifstream file(nomeArquivo);
    if (!file.is_open()) {
        cerr << "[ERRO] Não foi possível abrir o arquivo CSV: " << nomeArquivo << endl;
        return artigos;
    }

    string linha, acumulada;
    int aspasAbertas = 0;
    long registrosLidos = 0;
    long linhasConcatenadas = 0;

    while (getline(file, linha)) {
        if (linha.empty()) continue;

        if (!acumulada.empty()) acumulada += "\n"; // mantém o formato original
        acumulada += linha;
        aspasAbertas += contarAspas(linha);

        // Se ainda não fechou todas as aspas (número ímpar), continua acumulando
        if (aspasAbertas % 2 != 0) {
            linhasConcatenadas++;
            continue;
        }

        // Quando fecha (número par de aspas) → processa o registro completo
        vector<string> campos = dividirCampos(acumulada);

        if (campos.size() >= 7) {
            Artigo a;
            a.id = campos[0];
            a.titulo = campos[1];
            a.ano = campos[2];
            a.autores = campos[3];
            a.citacoes = campos[4];
            a.ultimaAtualizacao = campos[5];
            a.resumo = campos[6];
            artigos.push_back(a);
            registrosLidos++;
        } else {
            cerr << "[AVISO] Linha ignorada (campos insuficientes): "
                 << acumulada.substr(0, 80) << "...\n";
        }

        // Reseta para o próximo registro
        acumulada.clear();
        aspasAbertas = 0;
    }

    file.close();

    cout << "[INFO] Linhas concatenadas automaticamente: " << linhasConcatenadas << endl;
    cout << "[INFO] Total de artigos lidos: " << registrosLidos << endl;

    return artigos;
}
