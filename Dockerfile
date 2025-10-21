# Base Ubuntu 24.04
FROM ubuntu:24.04

# Instalação das ferramentas necessárias
RUN apt-get update && apt-get install -y \
    g++ make cmake && \
    rm -rf /var/lib/apt/lists/*

# Diretório de trabalho
WORKDIR /app

# Copia o conteúdo do projeto para dentro do contêiner
COPY . /app

# Compila os binários usando o Makefile
RUN make build

# Diretório para persistência de dados
VOLUME ["/data"]

# Variáveis de ambiente
ENV CSV_PATH=/data/input.csv \
    DATA_DIR=/data/db \
    LOG_LEVEL=info

# Comando padrão ao iniciar o contêiner
CMD ["bash", "-lc", "echo 'Use: docker run ... upload|findrec|seek1|seek2; ls -l bin/"]
