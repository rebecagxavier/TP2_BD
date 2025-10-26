#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>
#include "../include/BPlusTree.hpp"
#include "../include/HashFile.hpp"
#include "../include/Artigo.hpp"

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "[ERRO] Uso: seek2 \"<TITULO>\"" << endl;
        return 1;
    }

    string titulo = argv[1];
    // Permite espaços no título (caso o shell corte o argumento)
    for (int i = 2; i < argc; ++i) {
        titulo += " ";
        titulo += argv[i];
    }

    // Gera chave inteira via hash do título
    uint64_t chave = std::hash<std::string>{}(titulo);

    auto t0 = high_resolution_clock::now();

    // Abre índice secundário (por título)
    BPlusTree bpt("data/index_titulo.idx");

    // Busca hash(titulo) na árvore
    long offset = bpt.search(static_cast<int>(chave % INT_MAX)); // reduz para int
    int blocosLidos = bpt.getBlocosLidos();
    int totalBlocos = bpt.getTotalBlocos();

    if (offset == -1) {
        cout << "[INFO] Registro com título \"" << titulo << "\" não encontrado." << endl;
        cout << "[INFO] Blocos lidos no índice: " << blocosLidos
             << " | Total de blocos no índice: " << totalBlocos << endl;
    } else {
        ifstream dados("data/artigos.dat", ios::binary);
        if (!dados.is_open()) {
            cerr << "[ERRO] Não foi possível abrir data/artigos.dat" << endl;
            return 1;
        }

        dados.seekg(offset, ios::beg);

        short tamanho;
        if (!dados.read(reinterpret_cast<char*>(&tamanho), sizeof(short))) {
            cerr << "[ERRO] Falha ao ler tamanho do registro em offset " << offset << endl;
            return 1;
        }

        string registro(tamanho, '\0');
        if (!dados.read(&registro[0], tamanho)) {
            cerr << "[ERRO] Falha ao ler registro em offset " << offset << endl;
            return 1;
        }

        cout << "\n[OK] Registro encontrado!" << endl;
        cout << registro << endl;
        cout << "[INFO] Blocos lidos no índice: " << blocosLidos
             << " | Total de blocos no índice: " << totalBlocos << endl;
    }

    auto t1 = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(t1 - t0).count();
    cout << "[INFO] Tempo de execução: " << dur << " ms" << endl;

    bpt.close();
    return 0;
}
