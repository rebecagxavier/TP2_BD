#ifndef HASHFILE_HPP
#define HASHFILE_HPP

#include "Artigo.hpp"
#include <fstream>
#include <string>
#include <vector>

#define NUM_BUCKETS 524287 //número primo decidido de conforme tamanho do arquivo pra evitar colisões
#define TAM_BLOCO 4096 // tamanho lógico de bloco

int hashFunction(int id);
int stringToInt(const std::string &s);
void WriteVariableArtigo(std::ofstream &out, const Artigo &a);
void gravarHahing(const std::vector<Artigo> &artigos, const std::string &nomeArquivo);

#endif