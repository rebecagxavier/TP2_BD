#include "../include/BPlusTree.hpp"
#include <iostream>
#include <cstring>
#include <cassert>

using namespace std;

/*
 * Layout do arquivo de índice:
 * - Bloco 0 (offset 0): metadados
 *     long raizOffset;       // offset byte da raiz (-1 se vazio)
 *     long totalBlocos;      // número de blocos alocados no arquivo
 *     restante do bloco: zeros
 *
 * - Blocos seguintes (1..N): nós da B+Tree (cada nó ocupa BLOCK_SIZE bytes)
 *
 * Observações de implementação:
 * - posicaoNoArquivo de um nó é sempre um múltiplo de BLOCK_SIZE.
 * - alocarNovoBloco() escreve um bloco zero e retorna o offset onde o bloco começa.
 * - Esta implementação mantém os campos dinâmicos (vectors) em memória enquanto
 *   manipula um nó, e serializa/deserializa para um buffer de BLOCK_SIZE ao gravar/ler.
 * - MAX_KEYS deve ser escolhido de forma que a serialização caiba em BLOCK_SIZE.
 */

/* --------------------------
 * Construtor do nó
 * -------------------------- */
BPlusNode::BPlusNode(NodeType t) {
    tipo = t;
    numChaves = 0;
    proxFolha = -1;
    posicaoNoArquivo = -1;
}

/* --------------------------
 * Helpers privados - utilitários
 * -------------------------- */

/*
 * Inicializa o arquivo (se necessário) e carrega metadados.
 * Metadados ocupam o bloco 0:
 *   raizOffset (long) | totalBlocos (long) | padding
 */
BPlusTree::BPlusTree(const std::string& path) : filePath(path), raizOffset(-1), blocosLidos(0), totalBlocos(0) {
    // Abre ou cria o arquivo em modo leitura/escrita binário
    file.open(filePath, ios::in | ios::out | ios::binary);
    if (!file.is_open()) {
        // cria arquivo vazio
        file.open(filePath, ios::out | ios::binary);
        file.close();
        file.open(filePath, ios::in | ios::out | ios::binary);
    }

    // Se o arquivo estiver vazio (tamanho 0), inicializa o bloco de metadados
    file.seekg(0, ios::end);
    long fileSize = file.tellg();
    if (fileSize < BLOCK_SIZE) {
        // escreve bloco de metadados: raizOffset = -1, totalBlocos = 1 (já reservado o metadado)
        file.seekp(0, ios::beg);
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        long r = -1;
        long tb = 1; // já existe o bloco de metadados
        memcpy(buffer, &r, sizeof(long));
        memcpy(buffer + sizeof(long), &tb, sizeof(long));
        file.write(buffer, BLOCK_SIZE);
        file.flush();
        raizOffset = -1;
        totalBlocos = 1;
    } else {
        // carrega metadados
        file.seekg(0, ios::beg);
        char buffer[BLOCK_SIZE];
        file.read(buffer, BLOCK_SIZE);
        memcpy(&raizOffset, buffer, sizeof(long));
        memcpy(&totalBlocos, buffer + sizeof(long), sizeof(long));
        if (totalBlocos <= 0) totalBlocos = 1;
    }
}

/*
 * Destrutor garante que metadados sejam persistidos e o arquivo fechado.
 */
BPlusTree::~BPlusTree() {
    close();
}

/*
 * Fecha o arquivo e grava metadados atualizados no bloco 0.
 */
void BPlusTree::close() {
    if (!file.is_open()) return;

    // grava metadados no bloco 0
    file.seekp(0, ios::beg);
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    long r = raizOffset;
    long tb = totalBlocos;
    memcpy(buffer, &r, sizeof(long));
    memcpy(buffer + sizeof(long), &tb, sizeof(long));
    file.write(buffer, BLOCK_SIZE);
    file.flush();
    file.close();
}

/*
 * Aloca um novo bloco no final do arquivo.
 * Escreve BLOCK_SIZE bytes zerados e atualiza totalBlocos.
 * Retorna o offset (em bytes) do bloco recém-alocado.
 */
long BPlusTree::alocarNovoBloco() {
    file.seekp(0, ios::end);
    long offset = file.tellp();

    char vazio[BLOCK_SIZE];
    memset(vazio, 0, BLOCK_SIZE);
    file.write(vazio, BLOCK_SIZE);
    file.flush();

    totalBlocos = (offset / BLOCK_SIZE) + 1;
    return offset;
}

/*
 * Serialização / desserialização de nós:
 * - lerNo(offset): lê um bloco inteiro, desserializa para BPlusNode
 * - escreverNo(node): serializa e grava o bloco no posicaoNoArquivo do nó
 *
 * Formato no bloco:
 * [ NodeType (int) ][ numChaves (int) ]
 * [ chaves ... (numChaves * int) ]
 * Se INTERNAL:
 *   [ filhos ... ((numChaves+1) * long) ]
 * Se LEAF:
 *   [ offsets ... (numChaves * long) ][ proxFolha (long) ]
 *
 * O restante do bloco é padding com zeros.
 */

/*
 * Lê nó do offset (deve ser múltiplo de BLOCK_SIZE). Incrementa blocosLidos.
 */
BPlusNode BPlusTree::lerNo(long offset) {
    BPlusNode no(LEAF); // valor temporário, será sobrescrito
    assert(file.is_open());
    char buffer[BLOCK_SIZE];
    file.seekg(offset, ios::beg);
    file.read(buffer, BLOCK_SIZE);
    blocosLidos++;

    int pos = 0;
    int tipoInt = 0;
    memcpy(&tipoInt, buffer + pos, sizeof(int));
    pos += sizeof(int);
    no.tipo = (tipoInt == 0 ? INTERNAL : LEAF); // guarda como 0=INTERNAL,1=LEAF
    memcpy(&no.numChaves, buffer + pos, sizeof(int));
    pos += sizeof(int);

    // lê chaves
    no.chaves.resize(no.numChaves);
    for (int i = 0; i < no.numChaves; ++i) {
        memcpy(&no.chaves[i], buffer + pos, sizeof(int));
        pos += sizeof(int);
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

/*
 * Escreve nó no seu posicaoNoArquivo (deve estar definido).
 * Caso posicaoNoArquivo == -1, o nó não pode ser escrito (erro lógico).
 */
void BPlusTree::escreverNo(const BPlusNode& no) {
    assert(file.is_open());
    assert(no.posicaoNoArquivo != -1);

    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    int pos = 0;

    int tipoInt = (no.tipo == INTERNAL ? 0 : 1);
    memcpy(buffer + pos, &tipoInt, sizeof(int));
    pos += sizeof(int);

    memcpy(buffer + pos, &no.numChaves, sizeof(int));
    pos += sizeof(int);

    // chaves
    for (int i = 0; i < no.numChaves; ++i) {
        memcpy(buffer + pos, &no.chaves[i], sizeof(int));
        pos += sizeof(int);
    }

    if (no.tipo == INTERNAL) {
        // filhos: numChaves + 1
        for (int i = 0; i <= no.numChaves; ++i) {
            memcpy(buffer + pos, &no.filhos[i], sizeof(long));
            pos += sizeof(long);
        }
    } else {
        // offsets: numChaves
        for (int i = 0; i < no.numChaves; ++i) {
            memcpy(buffer + pos, &no.offsets[i], sizeof(long));
            pos += sizeof(long);
        }
        // ponteiro para próxima folha
        memcpy(buffer + pos, &no.proxFolha, sizeof(long));
        pos += sizeof(long);
    }

    file.seekp(no.posicaoNoArquivo, ios::beg);
    file.write(buffer, BLOCK_SIZE);
    file.flush();
}

/* --------------------------
 * Busca pública
 * -------------------------- */

/*
 * search(id): percorre a árvore a partir da raiz e retorna o offset no arquivo de dados,
 * ou -1 se não encontrar. Atualiza blocosLidos.
 */
long BPlusTree::search(int id) {
    blocosLidos = 0;
    if (raizOffset == -1) return -1;
    long atual = raizOffset;

    while (true) {
        BPlusNode no = lerNo(atual);
        if (no.tipo == INTERNAL) {
            // escolhe filho apropriado
            int i = 0;
            while (i < no.numChaves && id >= no.chaves[i]) i++;
            atual = no.filhos[i];
        } else {
            // folha: procura chave
            for (int i = 0; i < no.numChaves; ++i) {
                if (no.chaves[i] == id) {
                    return no.offsets[i];
                }
            }
            return -1;
        }
    }
}

/* --------------------------
 * Inserção (publica + recursiva)
 * -------------------------- */

/*
 * insert(id, offset):
 * - Se árvore vazia, cria uma folha raiz e insere.
 * - Senão, chama inserirRecursivo. Se ocorrer promoção (novoFilho != -1),
 *   cria nova raiz interna com a chave promovida.
 */
void BPlusTree::insert(int id, long offset) {
    blocosLidos = 0;

    // Árvore vazia: cria raiz folha
    if (raizOffset == -1) {
        BPlusNode raiz(LEAF);
        raiz.numChaves = 1;
        raiz.chaves = vector<int>(1, id);
        raiz.offsets = vector<long>(1, offset);
        raiz.proxFolha = -1;
        raiz.posicaoNoArquivo = alocarNovoBloco();
        escreverNo(raiz);
        raizOffset = raiz.posicaoNoArquivo;
        return;
    }

    long novoFilhoOffset = -1;
    int chavePromovida = -1;

    // chama recursivo sobre a raiz
    inserirRecursivo(id, offset, novoFilhoOffset, chavePromovida, raizOffset);

    // se houve promoção de chave para criar nova raiz
    if (novoFilhoOffset != -1) {
        BPlusNode novaRaiz(INTERNAL);
        novaRaiz.numChaves = 1;
        novaRaiz.chaves = vector<int>(1, chavePromovida);
        novaRaiz.filhos = vector<long>(2);
        novaRaiz.filhos[0] = raizOffset;
        novaRaiz.filhos[1] = novoFilhoOffset;
        novaRaiz.posicaoNoArquivo = alocarNovoBloco();
        escreverNo(novaRaiz);
        raizOffset = novaRaiz.posicaoNoArquivo;
    }
}

/*
 * inserirRecursivo:
 * - noAtual: offset do nó onde vamos inserir (ou descer)
 * - novoFilho / chavePromovida: retornam, quando um split acontece,
 *   o offset do novo nó à direita (novoFilho) e a chave a ser promovida
 *
 * Estratégia:
 * - Se leaf: insere ordenado. Se overflow (numChaves > MAX_KEYS), faz split de folha.
 * - Se internal: desce para filho correto; se filho retornou promoção, insere chave+ponteiro
 *   no nó atual; se overflow, faz split interno.
 */
void BPlusTree::inserirRecursivo(int id, long offset,
                                 long& novoFilho, int& chavePromovida,
                                 long noAtual) {
    novoFilho = -1;
    chavePromovida = -1;

    BPlusNode no = lerNo(noAtual);

    if (no.tipo == LEAF) {
        // encontra posição ordenada para inserir
        int i = 0;
        while (i < no.numChaves && id > no.chaves[i]) i++;
        no.chaves.insert(no.chaves.begin() + i, id);
        no.offsets.insert(no.offsets.begin() + i, offset);
        no.numChaves++;

        // sem overflow: grava e fim
        if (no.numChaves <= MAX_KEYS) {
            escreverNo(no);
            return;
        }

        // overflow -> split folha
        int meio = no.numChaves / 2;
        BPlusNode novaFolha(LEAF);
        int moved = no.numChaves - meio;
        novaFolha.numChaves = moved;
        novaFolha.chaves.assign(no.chaves.begin() + meio, no.chaves.end());
        novaFolha.offsets.assign(no.offsets.begin() + meio, no.offsets.end());
        novaFolha.proxFolha = no.proxFolha;
        novaFolha.posicaoNoArquivo = alocarNovoBloco();
        // ajusta a folha original
        no.numChaves = meio;
        no.chaves.resize(meio);
        no.offsets.resize(meio);
        no.proxFolha = novaFolha.posicaoNoArquivo;

        // escreve ambos
        escreverNo(no);
        escreverNo(novaFolha);

        // indica promoção: chavePromovida = primeiro key da nova folha
        novoFilho = novaFolha.posicaoNoArquivo;
        chavePromovida = novaFolha.chaves[0];
        return;
    }

    // no tipo INTERNAL: desce para filho correspondente
    int i = 0;
    while (i < no.numChaves && id >= no.chaves[i]) i++;
    long filhoOffset = no.filhos[i];

    long novoFilhoInterno = -1;
    int chavePromovidaInterna = -1;

    inserirRecursivo(id, offset, novoFilhoInterno, chavePromovidaInterna, filhoOffset);

    if (novoFilhoInterno == -1) {
        // filho não sofreu split
        return;
    }

    // inserimos a chavePromovidaInterna e o ponteiro novoFilhoInterno no nó atual
    int posInsert = 0;
    while (posInsert < no.numChaves && chavePromovidaInterna > no.chaves[posInsert]) posInsert++;
    no.chaves.insert(no.chaves.begin() + posInsert, chavePromovidaInterna);
    no.filhos.insert(no.filhos.begin() + posInsert + 1, novoFilhoInterno);
    no.numChaves++;

    // sem overflow
    if (no.numChaves <= MAX_KEYS) {
        escreverNo(no);
        return;
    }

    // overflow -> split nó interno
    int meio = no.numChaves / 2;
    BPlusNode novoInterno(INTERNAL);

    // chave a ser promovida para cima
    int chaveProm = no.chaves[meio];

    // novoInterno recebe as chaves à direita do meio (excluindo chaveProm)
    novoInterno.numChaves = no.numChaves - meio - 1;
    novoInterno.chaves.assign(no.chaves.begin() + meio + 1, no.chaves.end());
    novoInterno.filhos.assign(no.filhos.begin() + meio + 1, no.filhos.end());
    novoInterno.posicaoNoArquivo = alocarNovoBloco();

    // redimensiona o nó atual (esquerdo)
    no.numChaves = meio;
    no.chaves.resize(meio);
    no.filhos.resize(meio + 1);

    // escreve nós atualizados
    escreverNo(no);
    escreverNo(novoInterno);

    // sinaliza promoção para nível acima
    novoFilho = novoInterno.posicaoNoArquivo;
    chavePromovida = chaveProm;
}

/* --------------------------
 * Acesso a estatísticas
 * -------------------------- */

int BPlusTree::getBlocosLidos() const {
    return blocosLidos;
}

int BPlusTree::getTotalBlocos() const {
    return totalBlocos;
}
