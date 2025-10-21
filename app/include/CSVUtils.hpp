#ifndef CSVUTILS_HPP
#define CSVUTILS_HPP

#include <string>
#include <vector>
#include "Artigo.hpp"

std::string limparCampo(std::string campo); // Limpa e formata campo CSV
std::vector<Artigo> lerCSV(const std::string &nomeArquivo); // Lê arquivo CSV e retorna vetor de Artigos

#endif
