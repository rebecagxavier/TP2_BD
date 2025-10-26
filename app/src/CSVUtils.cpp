#include "../include/CSVUtils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

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

    campo.erase(remove(campo.begin(), campo.end(), '\r'), campo.end());
    campo.erase(remove(campo.begin(), campo.end(), '\n'), campo.end());
    return campo;
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
