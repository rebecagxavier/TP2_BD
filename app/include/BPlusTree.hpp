#ifndef BPLUSTREE_HPP
#define BPLUSTREE_HPP

#include <string>
#include <vector>
#include <fstream>

#define BLOCK_SIZE 4096
#define MAX_KEYS 64  // Ajustável conforme tamanho dos campos

enum NodeType { INTERNAL, LEAF };

//
// Estrutura de nó da B+Tree.
// Cada nó ocupa um bloco fixo de 4096 bytes no arquivo.
// Nós internos contêm chaves e ponteiros para filhos.
// Nós folha contêm pares (ID, offset) e ponteiro para próxima folha.
//
struct BPlusNode {
    NodeType tipo;              // Tipo do nó (folha ou interno)
    int numChaves;              // Número de chaves armazenadas
    std::vector<int> chaves;    // Chaves (IDs)

    // Para nós internos
    std::vector<long> filhos;   // Ponteiros para filhos

    // Para folhas
    std::vector<long> offsets;  // Offsets dos registros no arquivo de dados
    long proxFolha;             // Ponteiro para próxima folha (encadeamento)

    long posicaoNoArquivo;      // Offset do nó no arquivo de índice

    BPlusNode(NodeType t);      // Construtor
};

class BPlusTree {
private:
    std::fstream file;
    std::string filePath;
    long raizOffset;
    int blocosLidos;
    int totalBlocos;

    BPlusNode lerNo(long offset);
    void escreverNo(const BPlusNode& no);
    long alocarNovoBloco();
    void inserirRecursivo(int id, long offset, long& novoFilho, int& chavePromovida, long noAtual);

public:
    BPlusTree(const std::string& path);
    ~BPlusTree();

    void insert(int id, long offset); // Inserção com split
    long search(int id);              // Busca por ID

    int getBlocosLidos() const;
    int getTotalBlocos() const;
    void close();
};

#endif
