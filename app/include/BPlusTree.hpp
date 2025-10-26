#ifndef BPLUSTREE_HPP
#define BPLUSTREE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

#define BLOCK_SIZE 4096
#define MAX_KEYS 64  // Ajustável conforme tamanho dos campos

enum NodeType { INTERNAL, LEAF };

struct BPlusNode {
    NodeType tipo;              // Tipo do nó
    int numChaves;              // Número de chaves armazenadas
    std::vector<uint64_t> chaves; // Chaves (ID ou hash)
    
    std::vector<long> filhos;   // Filhos para nós internos
    std::vector<long> offsets;  // Offsets para folhas
    long proxFolha;             // Próxima folha

    long posicaoNoArquivo;      // Offset do nó no arquivo

    BPlusNode(NodeType t);
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
    void inserirRecursivo(uint64_t chave, long offset, long& novoFilho, uint64_t& chavePromovida, long noAtual);

public:
    BPlusTree(const std::string& path);
    ~BPlusTree();

    void insert(uint64_t chave, long offset);
    long search(uint64_t chave);

    int getBlocosLidos() const;
    int getTotalBlocos() const;
    void close();
};

#endif
