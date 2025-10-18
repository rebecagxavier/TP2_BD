#ifndef ARTIGO_HPP
#define ARTIGO_HPP

#include <string>

//struct com todos os tipos como string para facilitar na leitura e tratamento de dados inicial
struct Artigo { 
    std::string id;
    std::string titulo;
    std::string ano;
    std::string autores;
    std::string citacoes;
    std::string ultimaAtualizacao;
    std::string resumo;
};

#endif
