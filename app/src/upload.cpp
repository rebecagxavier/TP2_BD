// upload.cpp
// Lê CSV, grava arquivo de dados por hashing e gera índices primário e secundário (B+Tree em disco)

#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>
#include <climits>
#include "../include/CSVUtils.hpp"
#include "../include/HashFile.hpp"
#include "../include/BPlusTree.hpp"

using namespace std;
using namespace chrono;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: ./bin/upload <arquivo_csv>\n";
        return 1;
    }

    string csvPath = argv[1];
    string dataPath = "data/artigos.dat";

    auto inicio = high_resolution_clock::now();

    cout << "[INFO] Lendo CSV: " << csvPath << endl;
    vector<Artigo> artigos = lerCSV(csvPath);
    cout << "[INFO] Registros lidos: " << artigos.size() << endl;

    cout << "[INFO] Gravando arquivo de dados (hashing híbrido)...\n";
    gravarHashing(artigos, dataPath);

    // -------------------------------------------------------------
    // Gera índice primário (por ID) e secundário (por título)
    // -------------------------------------------------------------
    cout << "[INFO] Gerando índices B+Tree em disco..." << endl;

    fstream dados(dataPath, ios::in | ios::binary);
    if (!dados.is_open()) {
        cerr << "[ERRO] Não foi possível abrir " << dataPath << " para criar índices." << endl;
        return 1;
    }

    long headerSize = NUM_BUCKETS * sizeof(long);

    // Obtém tamanho do arquivo de dados
    dados.seekg(0, ios::end);
    long fileSize = dados.tellg();

    BPlusTree idxPrimario("data/index_primario.idx");
    BPlusTree idxTitulo("data/index_titulo.idx");

    long offset = headerSize;
    while (offset + static_cast<long>(sizeof(short)) <= fileSize) {
        dados.seekg(offset, ios::beg);

        short tamanho;
        if (!dados.read(reinterpret_cast<char*>(&tamanho), sizeof(short))) {
            cerr << "[WARN] Não foi possível ler tamanho em offset " << offset << endl;
            break;
        }

        if (tamanho <= 0 || offset + sizeof(short) + tamanho > fileSize) {
            cerr << "[WARN] Tamanho inválido em offset " << offset << ": " << tamanho << endl;
            break;
        }

        string registro(tamanho, '\0');
        if (!dados.read(&registro[0], tamanho)) {
            cerr << "[WARN] Falha ao ler registro em offset " << offset << endl;
            break;
        }

        // Extrai ID
        size_t p1 = registro.find(';');
        if (p1 != string::npos) {
            string idStr = registro.substr(0, p1);
            int idInt = stringToInt(idStr);

            if (idInt > 0) {
                // Debug opcional
                // cout << "[DEBUG] ID: " << idInt << " | Offset: " << offset << " | Tamanho: " << tamanho << endl;
                idxPrimario.insert(idInt, offset);
            }
        }

        // Extrai título
        size_t p2 = registro.find(';', p1 + 1);
        if (p1 != string::npos && p2 != string::npos) {
            string titulo = registro.substr(p1 + 1, p2 - p1 - 1);
            uint64_t hashTitulo = std::hash<std::string>{}(titulo);
            idxTitulo.insert(static_cast<int>(hashTitulo % INT_MAX), offset);
        }

        // Incrementa offset: tamanho válido + ponteiro long (proxOffset)
        offset += sizeof(short) + tamanho + sizeof(long);

        // Segurança: não ultrapassar fileSize
        if (offset >= fileSize) break;
    }

    idxPrimario.close();
    idxTitulo.close();
    dados.close();

    cout << "[INFO] Índices gerados:" << endl;
    cout << "       → data/index_primario.idx (por ID)" << endl;
    cout << "       → data/index_titulo.idx (por título)" << endl;

    auto fim = high_resolution_clock::now();
    auto duracao = duration_cast<milliseconds>(fim - inicio).count();

    cout << "[INFO] Tempo total de execução: " << duracao << " ms" << endl;
    cout << "[INFO] Arquivo de dados: " << dataPath << endl;

    return 0;
}
