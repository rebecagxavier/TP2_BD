CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
BIN_DIR = bin

# Fontes por programa 
# Essas variáveis agrupam os arquivos .cpp necessários para compilar cada programa
UPLOAD_SRC = src/upload.cpp src/CSVUtils.cpp src/HashFile.cpp src/BPlusTree.cpp
FINDREC_SRC = src/findrec.cpp src/HashFile.cpp src/CSVUtils.cpp #CSVUtils msm?______
SEEK1_SRC = src/seek1.cpp src/BPlusTree.cpp src/HashFile.cpp

# Compilação local
# Cada linha compila um programa e gera um executável dentro de bin/
build:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(UPLOAD_SRC) -o $(BIN_DIR)/upload
	$(CXX) $(CXXFLAGS) $(FINDREC_SRC) -o $(BIN_DIR)/findrec
	$(CXX) $(CXXFLAGS) $(SEEK1_SRC) -o $(BIN_DIR)/seek1

# Docker build
docker-build:
	docker build -t tp2 .

# Docker run para cada programa
docker-run-upload:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/upload /app/data/teste.csv

docker-run-findrec:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/findrec /app/data/artigos.dat 12

docker-run-seek1:
	docker run --rm -v $(PWD)/data:/app/data tp2 ./bin/seek1 9

# Limpeza dos binários
clean:
	rm -rf $(BIN_DIR)/*