#include "../include/CSVUtils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
using namespace std;

string limparCampo(string campo) {

    //trata aspas iniciais e finais
    if (!campo.empty() && campo.front() == '"')
        campo.erase(0, 1);
    if (!campo.empty() && campo.back() == '"')
        campo.pop_back();
   
    // Substituições HTML comuns
    const vector<pair<string, string>> htmlSubs = {
        {"&iacute;", "í"}, {"&aacute;", "á"}, {"&eacute;", "é"},
        {"&oacute;", "ó"}, {"&uacute;", "ú"}, {"&ccedil;", "ç"},
        {"<i>", ""}, {"</i>", ""}, {"<b>", ""}, {"</b>", ""}
    };

    //substitui os códigos HTML
    for (auto &p : htmlSubs) {
        size_t pos;
        while ((pos = campo.find(p.first)) != string::npos)
            campo.replace(pos, p.first.length(), p.second);
    }
    // Remove quebras de linha e retornos de carro
    string limpa;
    for (char c : campo) {
        if (c != '\r' && c != '\n')
            limpa += c;
        }
        campo = limpa;
    return campo;
}

vector<Artigo> lerCSV(const string &nomeArquivo) {

    vector<Artigo> artigos;
    ifstream file(nomeArquivo); // Abre o arquivo CSV

    if (!file.is_open()) {
        cerr << "[ERRO] Não foi possível abrir o arquivo CSV: " << nomeArquivo << endl;
        return artigos;
    }

    string linha;
    while (getline(file, linha)) {
        
        if (linha.empty()) continue;

        vector<string> campos; // Armazena os campos do artigo
        size_t pos = 0;

        // Processa campos usando o delimitador ' ";" '
        while ((pos = linha.find("\";\"")) != string::npos) {
            string token = linha.substr(0, pos + 1);
            campos.push_back(limparCampo(token));
            linha.erase(0, pos + 3); // remove o delimitador (";")
        }

        // Trata o último campo
        if (!linha.empty()) {
            // Pode estar entre aspas, sem aspas ou ser NULL
            string ultimo = limparCampo(linha);
            campos.push_back(ultimo);
        }

        // Se ainda tiver menos de 7 campos, completa com vazio
        while (campos.size() < 7)
            campos.push_back("");

        Artigo a;
        a.id = campos[0];
        a.titulo = campos[1];
        a.ano = campos[2];
        a.autores = campos[3];
        a.citacoes = campos[4];
        a.ultimaAtualizacao = campos[5];
        a.resumo = campos[6];

        artigos.push_back(a);
    }

    file.close();
    return artigos;
}

