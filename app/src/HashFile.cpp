#include "../include/HashFile.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

int stringToInt(const string &s) {
    try {
        if (s.empty()) return 0;
        return stoi(s);
    } catch (...) {
        return 0;
    }
}

// Função hash simples
int hashFunction(int id) {
    return id % NUM_BUCKETS;
}

void WriteVariableArtigo(fstream &out, const Artigo &a, long proxOffset) {
    string registro = a.id + ";" + a.titulo + ";" + a.ano + ";" +
                      a.autores + ";" + a.citacoes + ";" +
                      a.ultimaAtualizacao + ";" + a.resumo + ";";

    short length = registro.size();

    // escreve tamanho
    out.write(reinterpret_cast<char*>(&length), sizeof(length));
    // escreve dados
    out.write(registro.c_str(), length);
    // escreve ponteiro para próximo
    out.write(reinterpret_cast<char*>(&proxOffset), sizeof(proxOffset));
}

long lerProxOffset(fstream &file, long offset) {
    file.seekg(offset, ios::beg); // vai para o offset do registro
    short length;
    if (!file.read(reinterpret_cast<char*>(&length), sizeof(length)))
        return -1; // leitura inválida

    // pula o registro
    file.seekg(length, ios::cur);

    long prox;
    if (!file.read(reinterpret_cast<char*>(&prox), sizeof(prox)))
        return -1; 

    return prox;
}

void gravarHashing(const vector<Artigo> &artigos, const string &nomeArquivo) {
    fstream out(nomeArquivo, ios::binary | ios::in | ios::out);
    if (!out.is_open()) {
    // se não existir, cria
        out.open(nomeArquivo, ios::binary | ios::out);
        out.close();
        out.open(nomeArquivo, ios::binary | ios::in | ios::out);
        //return;
    }

    // inicializa área de cabeçalho (vetor de buckets)
    vector<long> bucketOffsets(NUM_BUCKETS, -1);
    long headerSize = NUM_BUCKETS * sizeof(long);

    // escreve cabeçalho vazio
    out.seekp(0, ios::beg);
    out.write(reinterpret_cast<char*>(bucketOffsets.data()), headerSize); // espaço para o cabeçalho

    size_t blocosEscritos = 0;

    for (const auto &a : artigos) {
        int idInt = stringToInt(a.id);
        int anoInt = stringToInt(a.ano);
        int citacoesInt = stringToInt(a.citacoes);
        if (idInt == 0) {
            cerr << "[AVISO] Ignorando registro com ID inválido: " << a.id << endl;
            continue;
        }

        int pos = hashFunction(idInt);
        long headOffset = headerSize + pos * TAM_BLOCO; // posição inicial do bucket
        long novoOffset;

        // Posiciona o ponteiro no fim do arquivo (para gravar o novo registro)
        out.seekp(0, ios::end);
        novoOffset = out.tellp();

        // Escreve o novo registro com proxOffset = -1
        WriteVariableArtigo(out, a, -1);
        blocosEscritos++;

        // Agora liga esse novo registro à lista encadeada do bucket
        if (bucketOffsets[pos] == -1) {
            // bucket vazio → atualiza direto no cabeçalho
            bucketOffsets[pos] = novoOffset;
        } else {
            // bucket já ocupado → percorre até o final da lista
            long atual = bucketOffsets[pos];
            long prox = lerProxOffset(out, atual);

            while (prox != -1) {
                atual = prox;
                prox = lerProxOffset(out, atual);
            }

            // atualiza o campo proxOffset do último registro
            out.seekp(atual, ios::beg);
            short len;
            out.read(reinterpret_cast<char*>(&len), sizeof(len));
            out.seekp(len, ios::cur);
            out.write(reinterpret_cast<char*>(&novoOffset), sizeof(novoOffset));
        }
    }

    // regrava o cabeçalho atualizado no início
    out.seekp(0, ios::beg);
    out.write(reinterpret_cast<char*>(bucketOffsets.data()), headerSize);
    out.close();

    cout << "[INFO] " << blocosEscritos << " registros gravados em " << nomeArquivo << endl;
    cout << "[INFO] Buckets: " << NUM_BUCKETS << " | Bloco lógico: " << TAM_BLOCO << " bytes" << endl;
}
