comandos:
g++ -std=c++17 upload.cpp CSVUtils.cpp HashFile.cpp -I. -o upload
 ./upload arquivo.csv
g++ -std=c++17 src/findrec.cpp src/HashFile.cpp -o search
 ./search data/artigos.dat IDbuscado

    
    Comandos docker disponíveis:

    make build              → Compila os binários localmente"
    make docker-build       → Constrói a imagem Docker"
    make docker-run-upload  → Executa o programa upload no contêiner"
    make docker-run-findrec → Executa o programa findrec no contêiner"
    make docker-run-seek1   → Executa o programa seek1 no contêiner"
    make docker-run-seek2   → Executa o programa seek2 no contêiner"