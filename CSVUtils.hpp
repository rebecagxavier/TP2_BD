#ifndef CSVUTILS_HPP
#define CSVUTILS_HPP

#include <string>
#include <vector>
#include "Artigo.hpp"

std::string limparCampo(std::string campo);
std::vector<Artigo> lerCSV(const std::string &nomeArquivo);

#endif
