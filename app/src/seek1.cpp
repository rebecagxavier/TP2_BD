#include <iostream>
#include <fstream>
#include <chrono>
#include "../include/BPlusTree.hpp"
#include "../include/HashFile.hpp"
#include "../include/Artigo.hpp"

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "[ERRO] Uso: seek1 <ID>" << endl;
        return 1;
    }

    int id = stringToInt(argv[1]);

    auto t0 = high_resolution_clock::now();

    // Abre índice primário em disco
    BPlusTree bpt("data/index_primario.idx");

    // Faz a busca no índice (retorna offset no arquivo de dados ou -1)
    long offset = bpt.search(id);
    int blocosLidos = bpt.getBlocosLidos();
    int totalBlocos = bpt.getTotalBlocos();

    if (offset == -1) {
        cout << "[INFO] Registro com ID " << id << " não encontrado." << endl;
        cout << "[INFO] Blocos lidos no índice: " << blocosLidos << " | Total de blocos no índice: " << totalBlocos << endl;
    } else {
        // Abre o arquivo de dados e lê o registro no offset encontrado
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

        // O formato do registro é: "id;titulo;ano;autores;citacoes;ultimaAtualizacao;resumo;"
        cout << "\n[OK] Registro encontrado!" << endl;
        cout << registro << '\n' << flush; //<< endl;
        cout << "[INFO] Blocos lidos no índice: " << blocosLidos << " | Total de blocos no índice: " << totalBlocos << endl;
    }

    auto t1 = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(t1 - t0).count();
    cout << "[INFO] Tempo de execução: " << dur << " ms" << endl;

    bpt.close();
    return 0;
}
