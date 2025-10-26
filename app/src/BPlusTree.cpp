#include "../include/BPlusTree.hpp"
#include <iostream>
#include <cstring>
#include <cassert>

using namespace std;

BPlusNode::BPlusNode(NodeType t) {
    tipo = t;
    numChaves = 0;
    proxFolha = -1;
    posicaoNoArquivo = -1;
}

/* -------------------------- Helpers -------------------------- */

BPlusTree::BPlusTree(const string& path)
    : filePath(path), raizOffset(-1), blocosLidos(0), totalBlocos(0)
{
    file.open(filePath, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        file.open(filePath, ios::out | ios::binary);
        file.close();
        file.open(filePath, ios::in | ios::out | ios::binary);
    }

    file.seekg(0, ios::end);
    long fileSize = file.tellg();
    if (fileSize < BLOCK_SIZE) {
        char buffer[BLOCK_SIZE]{0};
        long r = -1;
        long tb = 1;
        memcpy(buffer, &r, sizeof(long));
        memcpy(buffer + sizeof(long), &tb, sizeof(long));
        file.seekp(0, ios::beg);
        file.write(buffer, BLOCK_SIZE);
        file.flush();
        raizOffset = -1;
        totalBlocos = 1;
    } else {
        char buffer[BLOCK_SIZE];
        file.seekg(0, ios::beg);
        file.read(buffer, BLOCK_SIZE);
        memcpy(&raizOffset, buffer, sizeof(long));
        memcpy(&totalBlocos, buffer + sizeof(long), sizeof(long));
        if (totalBlocos <= 0) totalBlocos = 1;
    }
}

BPlusTree::~BPlusTree() { close(); }

void BPlusTree::close() {
    if (!file.is_open()) return;
    char buffer[BLOCK_SIZE]{0};
    memcpy(buffer, &raizOffset, sizeof(long));
    memcpy(buffer + sizeof(long), &totalBlocos, sizeof(long));
    file.seekp(0, ios::beg);
    file.write(buffer, BLOCK_SIZE);
    file.flush();
    file.close();
}

long BPlusTree::alocarNovoBloco() {
    file.seekp(0, ios::end);
    long offset = file.tellp();
    char vazio[BLOCK_SIZE]{0};
    file.write(vazio, BLOCK_SIZE);
    file.flush();
    totalBlocos = (offset / BLOCK_SIZE) + 1;
    return offset;
}

/* -------------------------- Serialização -------------------------- */

BPlusNode BPlusTree::lerNo(long offset) {
    BPlusNode no(LEAF);
    char buffer[BLOCK_SIZE];
    file.seekg(offset, ios::beg);
    file.read(buffer, BLOCK_SIZE);
    blocosLidos++;

    int pos = 0;
    int tipoInt;
    memcpy(&tipoInt, buffer + pos, sizeof(int));
    pos += sizeof(int);
    no.tipo = (tipoInt == 0 ? INTERNAL : LEAF);

    memcpy(&no.numChaves, buffer + pos, sizeof(int));
    pos += sizeof(int);

    no.chaves.resize(no.numChaves);
    for (int i = 0; i < no.numChaves; ++i) {
        memcpy(&no.chaves[i], buffer + pos, sizeof(uint64_t));
        pos += sizeof(uint64_t);
    }

    if (no.tipo == INTERNAL) {
        no.filhos.resize(no.numChaves + 1);
        for (int i = 0; i <= no.numChaves; ++i) {
            memcpy(&no.filhos[i], buffer + pos, sizeof(long));
            pos += sizeof(long);
        }
    } else {
        no.offsets.resize(no.numChaves);
        for (int i = 0; i < no.numChaves; ++i) {
            memcpy(&no.offsets[i], buffer + pos, sizeof(long));
            pos += sizeof(long);
        }
        memcpy(&no.proxFolha, buffer + pos, sizeof(long));
        pos += sizeof(long);
    }

    no.posicaoNoArquivo = offset;
    return no;
}

void BPlusTree::escreverNo(const BPlusNode& no) {
    assert(no.posicaoNoArquivo != -1);
    char buffer[BLOCK_SIZE]{0};
    int pos = 0;

    int tipoInt = (no.tipo == INTERNAL ? 0 : 1);
    memcpy(buffer + pos, &tipoInt, sizeof(int));
    pos += sizeof(int);

    memcpy(buffer + pos, &no.numChaves, sizeof(int));
    pos += sizeof(int);

    for (int i = 0; i < no.numChaves; ++i) {
        memcpy(buffer + pos, &no.chaves[i], sizeof(uint64_t));
        pos += sizeof(uint64_t);
    }

    if (no.tipo == INTERNAL) {
        for (int i = 0; i <= no.numChaves; ++i) {
            memcpy(buffer + pos, &no.filhos[i], sizeof(long));
            pos += sizeof(long);
        }
    } else {
        for (int i = 0; i < no.numChaves; ++i) {
            memcpy(buffer + pos, &no.offsets[i], sizeof(long));
            pos += sizeof(long);
        }
        memcpy(buffer + pos, &no.proxFolha, sizeof(long));
    }

    file.seekp(no.posicaoNoArquivo, ios::beg);
    file.write(buffer, BLOCK_SIZE);
    file.flush();
}

/* -------------------------- Busca -------------------------- */

long BPlusTree::search(uint64_t chave) {
    blocosLidos = 0;
    if (raizOffset == -1) return -1;
    long atual = raizOffset;

    while (true) {
        BPlusNode no = lerNo(atual);
        if (no.tipo == INTERNAL) {
            int i = 0;
            while (i < no.numChaves && chave >= no.chaves[i]) i++;
            atual = no.filhos[i];
        } else {
            for (int i = 0; i < no.numChaves; ++i) {
                if (no.chaves[i] == chave) return no.offsets[i];
            }
            return -1;
        }
    }
}

/* -------------------------- Inserção -------------------------- */

void BPlusTree::insert(uint64_t chave, long offset) {
    if (raizOffset == -1) {
        BPlusNode raiz(LEAF);
        raiz.numChaves = 1;
        raiz.chaves.push_back(chave);
        raiz.offsets.push_back(offset);
        raiz.posicaoNoArquivo = alocarNovoBloco();
        raiz.proxFolha = -1;
        escreverNo(raiz);
        raizOffset = raiz.posicaoNoArquivo;
        return;
    }

    long novoFilho = -1;
    uint64_t chavePromovida = 0;
    inserirRecursivo(chave, offset, novoFilho, chavePromovida, raizOffset);

    if (novoFilho != -1) {
        BPlusNode novaRaiz(INTERNAL);
        novaRaiz.numChaves = 1;
        novaRaiz.chaves.push_back(chavePromovida);
        novaRaiz.filhos = { raizOffset, novoFilho };
        novaRaiz.posicaoNoArquivo = alocarNovoBloco();
        escreverNo(novaRaiz);
        raizOffset = novaRaiz.posicaoNoArquivo;
    }
}

void BPlusTree::inserirRecursivo(uint64_t chave, long offset,
                                 long& novoFilho, uint64_t& chavePromovida,
                                 long noAtual)
{
    novoFilho = -1;
    chavePromovida = 0;

    BPlusNode no = lerNo(noAtual);

    if (no.tipo == LEAF) {
        int i = 0;
        while (i < no.numChaves && chave > no.chaves[i]) i++;
        no.chaves.insert(no.chaves.begin() + i, chave);
        no.offsets.insert(no.offsets.begin() + i, offset);
        no.numChaves++;

        if (no.numChaves <= MAX_KEYS) {
            escreverNo(no);
            return;
        }

        // split folha
        int meio = no.numChaves / 2;
        BPlusNode novaFolha(LEAF);
        int moved = no.numChaves - meio;
        novaFolha.numChaves = moved;
        novaFolha.chaves.assign(no.chaves.begin() + meio, no.chaves.end());
        novaFolha.offsets.assign(no.offsets.begin() + meio, no.offsets.end());
        novaFolha.proxFolha = no.proxFolha;
        novaFolha.posicaoNoArquivo = alocarNovoBloco();

        no.numChaves = meio;
        no.chaves.resize(meio);
        no.offsets.resize(meio);
        no.proxFolha = novaFolha.posicaoNoArquivo;

        escreverNo(no);
        escreverNo(novaFolha);

        novoFilho = novaFolha.posicaoNoArquivo;
        chavePromovida = novaFolha.chaves[0];
        return;
    }

    // internal node
    int i = 0;
    while (i < no.numChaves && chave >= no.chaves[i]) i++;
    long filhoOffset = no.filhos[i];

    long novoFilhoInterno = -1;
    uint64_t chavePromovidaInterna = 0;

    inserirRecursivo(chave, offset, novoFilhoInterno, chavePromovidaInterna, filhoOffset);

    if (novoFilhoInterno == -1) return;

    // insere chave promovida
    int posInsert = 0;
    while (posInsert < no.numChaves && chavePromovidaInterna > no.chaves[posInsert]) posInsert++;
    no.chaves.insert(no.chaves.begin() + posInsert, chavePromovidaInterna);
    no.filhos.insert(no.filhos.begin() + posInsert + 1, novoFilhoInterno);
    no.numChaves++;

    if (no.numChaves <= MAX_KEYS) {
        escreverNo(no);
        return;
    }

    // split interno
    int meio = no.numChaves / 2;
    BPlusNode novoInterno(INTERNAL);
    uint64_t chaveProm = no.chaves[meio];

    novoInterno.numChaves = no.numChaves - meio - 1;
    novoInterno.chaves.assign(no.chaves.begin() + meio + 1, no.chaves.end());
    novoInterno.filhos.assign(no.filhos.begin() + meio + 1, no.filhos.end());
    novoInterno.posicaoNoArquivo = alocarNovoBloco();

    no.numChaves = meio;
    no.chaves.resize(meio);
    no.filhos.resize(meio + 1);

    escreverNo(no);
    escreverNo(novoInterno);

    novoFilho = novoInterno.posicaoNoArquivo;
    chavePromovida = chaveProm;
}

/* -------------------------- Estatísticas -------------------------- */

int BPlusTree::getBlocosLidos() const { return blocosLidos; }
int BPlusTree::getTotalBlocos() const { return totalBlocos; }
