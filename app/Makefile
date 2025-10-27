CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
BIN_DIR = bin

# Fontes por programa
UPLOAD_SRC = src/upload.cpp src/CSVUtils.cpp src/HashFile.cpp src/BPlusTree.cpp
FINDREC_SRC = src/findrec.cpp src/HashFile.cpp src/CSVUtils.cpp
SEEK1_SRC = src/seek1.cpp src/BPlusTree.cpp src/HashFile.cpp
SEEK2_SRC = src/seek2.cpp src/BPlusTree.cpp src/CSVUtils.cpp

# Variáveis dinâmicas
ID ?= 0
TITLE ?=

# Compilação local
build:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(UPLOAD_SRC) -o $(BIN_DIR)/upload
	$(CXX) $(CXXFLAGS) $(FINDREC_SRC) -o $(BIN_DIR)/findrec
	$(CXX) $(CXXFLAGS) $(SEEK1_SRC) -o $(BIN_DIR)/seek1
	$(CXX) $(CXXFLAGS) $(SEEK2_SRC) -o $(BIN_DIR)/seek2

# Docker build
docker-build:
	docker build -t tp2 .

# Docker run com parâmetros
docker-run-upload:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/upload /app/data/input.csv

docker-run-findrec:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/findrec /app/data/artigos.dat $(ID)

docker-run-seek1:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/seek1 $(ID)

#docker-run-seek2:
    docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/seek2 "$(TITLE)"

# Limpeza dos binários
clean:
	rm -rf $(BIN_DIR)/*

.PHONY: build docker-build docker-run-upload docker-run-findrec docker-run-seek1 docker-run-seek2 clean
