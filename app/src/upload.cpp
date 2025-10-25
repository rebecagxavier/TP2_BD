// Lê CSV, grava arquivo de dados por hashing e gera índice primário (B+Tree em disco)

#include <iostream>
#include <chrono>
#include <fstream>
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

    // ----------------------------
    // Gera índice primário (B+Tree em disco)
    // ----------------------------
    cout << "[INFO] Gerando índice primário (B+Tree em disco)..." << endl;

    // abre arquivo de dados para ler offsets e extrair IDs
    fstream dados(dataPath, ios::in | ios::binary);
    if (!dados.is_open()) {
        cerr << "[ERRO] Não foi possível abrir " << dataPath << " para criar índice." << endl;
    } else {
        // cria/abre arquivo de índice
        BPlusTree bpt("data/index_primario.idx");

        // cabeçalho do arquivo de hashing: NUM_BUCKETS * sizeof(long)
        long headerSize = NUM_BUCKETS * sizeof(long);

        // obtenha o tamanho do arquivo de dados
        dados.seekg(0, ios::end);
        long fileSize = dados.tellg();

        // percorre registros a partir do primeiro possível offset (após cabeçalho)
        long offset = headerSize;
        while (offset + static_cast<long>(sizeof(short)) <= fileSize) {
            dados.seekg(offset, ios::beg);

            // lê o tamanho do registro
            short tamanho;
            if (!dados.read(reinterpret_cast<char*>(&tamanho), sizeof(short))) break;

            // proteção contra tamanho inválido
            if (tamanho <= 0 || offset + sizeof(short) + tamanho > fileSize) break;

            // lê o registro (conteúdo)
            string registro(tamanho, '\0');
            if (!dados.read(&registro[0], tamanho)) break;

            // extrai ID (antes do primeiro ';')
            size_t delim = registro.find(';');
            if (delim != string::npos) {
                string idStr = registro.substr(0, delim);
                int idInt = stringToInt(idStr);
                if (idInt > 0) {
                    // insere (ID, offset) na B+Tree
                    bpt.insert(idInt, offset);
                }
            }

            // pula o ponteiro proxOffset (long) posicionado logo após o registro
            offset += sizeof(short) + tamanho + sizeof(long);
        }

        bpt.close();
        dados.close();
        cout << "[INFO] Índice primário gerado em: data/index_primario.idx" << endl;
    }

    auto fim = high_resolution_clock::now();
    auto duracao = duration_cast<milliseconds>(fim - inicio).count();

    cout << "[INFO] Tempo total de execução: " << duracao << " ms" << endl;
    cout << "[INFO] Arquivo de dados gerado em: " << dataPath << endl;

    return 0;
}
