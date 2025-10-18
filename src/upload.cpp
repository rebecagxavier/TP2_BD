#include <iostream>
#include <chrono>
#include "../include/CSVUtils.hpp"
#include "../include/HashFile.hpp"

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
    cout << "[INFO*] Registros lidos: " << artigos.size() << endl;

    cout << "[INFO] Gravando arquivo de dados (hashing híbrido)...\n";
    gravarHashing(artigos, dataPath);

    auto fim = high_resolution_clock::now();
    auto duracao = duration_cast<milliseconds>(fim - inicio).count();

    cout << "[INFO] Tempo total de execução: " << duracao << " ms" << endl;
    cout << "[INFO] Arquivo de dados gerado em: " << dataPath << endl;

    return 0;
}
