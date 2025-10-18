#ifndef HASHFILE_HPP
#define HASHFILE_HPP

#include "Artigo.hpp"
#include <fstream>
#include <string>
#include <vector>

#define NUM_BUCKETS 524287 //número primo decidido de conforme tamanho do arquivo pra evitar colisões
#define TAM_BLOCO 4096 // tamanho lógico de bloco

int hashFunction(int id); // Função de hash
int stringToInt(const std::string &s); // Conversão segura string para int
void WriteVariableArtigo(std::ofstream &out, const Artigo &a); // Escrita de registro variável
void gravarHashing(const std::vector<Artigo> &artigos, const std::string &nomeArquivo); // Gravação por hashing híbrido

#endif