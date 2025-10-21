CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
BIN_DIR = bin

# Fontes por programa 
# Essas variáveis agrupam os arquivos .cpp necessários para compilar cada programa
UPLOAD_SRC = src/upload.cpp src/BPTree.cpp src/CSVUtils.cpp
FINDREC_SRC = src/findrec.cpp src/HashFile.cpp src/CSVUtils.cpp
SEEK1_SRC = src/seek1.cpp src/BPTree.cpp src/CSVUtils.cpp
SEEK2_SRC = src/seek2.cpp src/BPTree.cpp src/CSVUtils.cpp

# Compilação local
# Cada linha compila um programa e gera um executável dentro de bin/
build:
    mkdir -p $(BIN_DIR)
    $(CXX) $(CXXFLAGS) $(UPLOAD_SRC) -o $(BIN_DIR)/upload
    $(CXX) $(CXXFLAGS) $(FINDREC_SRC) -o $(BIN_DIR)/findrec
    $(CXX) $(CXXFLAGS) $(SEEK1_SRC) -o $(BIN_DIR)/seek1
    $(CXX) $(CXXFLAGS) $(SEEK2_SRC) -o $(BIN_DIR)/seek2

# Docker build
docker-build:
    docker build -t tp2 .

# Docker run para cada programa
docker-run-upload:
    docker run --rm -v $(PWD)/data:/data tp2 ./bin/upload /data/input.csv

docker-run-findrec:
    docker run --rm -v $(PWD)/data:/data tp2 ./bin/findrec 123

docker-run-seek1:
    docker run --rm -v $(PWD)/data:/data tp2 ./bin/seek1 123

docker-run-seek2:
    docker run --rm -v $(PWD)/data:/data tp2 ./bin/seek2 "Um Título Exato"
