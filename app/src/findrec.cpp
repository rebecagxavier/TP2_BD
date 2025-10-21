#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "../include/HashFile.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: ./search <arquivo_dat> <id_para_buscar>\n";
        return 1;
    }

    string dataPath = argv[1];
    int id = stoi(argv[2]);

    fstream in(dataPath, ios::binary | ios::in);
    if (!in.is_open()) {
        cerr << "[ERRO] Não foi possível abrir o arquivo de dados.\n";
        return 1;
    }

    // calcula informações do arquivo
    in.seekg(0, ios::end);
    long fileSize = in.tellg();
    in.seekg(0, ios::beg);

    // calcula o tamanho do cabeçalho e quantidade de blocos totais
    long headerSize = NUM_BUCKETS * sizeof(long);
    long totalBlocos = (fileSize - headerSize) / TAM_BLOCO;
    if (totalBlocos <= 0) totalBlocos = 1;

    // lê o vetor de ponteiros do cabeçalho
    vector<long> bucketOffsets(NUM_BUCKETS);
    in.read(reinterpret_cast<char*>(bucketOffsets.data()), headerSize);

    int pos = hashFunction(id);
    long offsetAtual = bucketOffsets[pos];

    if (offsetAtual == -1) {
        cout << "[INFO] Registro com ID " << id << " não encontrado.\n";
        cout << "[INFO] Blocos lidos: 0 | Total de blocos: " << totalBlocos << endl;
        return 0;
    }

    int blocosLidos = 0;

    // percorre a lista encadeada do bucket
    while (offsetAtual != -1) {
        
        blocosLidos++;
        in.seekg(offsetAtual, ios::beg);
        short length;

        if (!in.read(reinterpret_cast<char*>(&length), sizeof(length))) break;

        string registro(length, '\0');
        in.read(&registro[0], length); // lê o registro
        long proxOffset;
        in.read(reinterpret_cast<char*>(&proxOffset), sizeof(proxOffset)); // lê o ponteiro para o próximo

        // extrai o ID do registro (antes do primeiro ';')
        size_t delim = registro.find(';');
        string idLido = (delim != string::npos) ? registro.substr(0, delim) : "";

        if (stringToInt(idLido) == id) {
            cout << "\n[OK] Registro encontrado!\n";
            cout << registro << endl;
            cout << "[INFO] Blocos lidos: " << blocosLidos << endl;
            cout << "[INFO] Total de blocos no arquivo: " << totalBlocos << endl;
            in.close();
            return 0;
        }

        offsetAtual = proxOffset;
    }

    cout << "[INFO] Registro com ID " << id << " não encontrado.\n";
    cout << "[INFO] Blocos lidos: " << blocosLidos << " | Total de blocos: " << totalBlocos << endl;

    in.close();
    return 0;
}
