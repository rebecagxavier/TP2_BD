📦 TP2 — Instruções & Comandos

📥 Preparando o arquivo CSV de entrada
Antes de executar os programas, é necessário garantir que o arquivo input.csv esteja presente na pasta data/.
Para isso, baixe esse artigo.csv.gz que consta no link abaixo, descompacte, nomeie como "input.csv" e o coloque em TP2_BD/app/data/ :)

https://drive.google.com/file/d/1EVoP0d9Wwzj1O6eoFIkel9I3cpe43Gbv/view?usp=sharing

⚙️ Compilação e execução local

1. Compile os binários:
   ```bash
   make build

2. Execute o upload com o CSV:
   ```bash
   ./bin/upload ./data/input.csv

4. Busque por ID diretamente no arquivo:
   ```bash
   ./bin/findrec ./data/artigos.dat 230

4.Busque por título (índice secundário):
   
   ./bin/seek2 "Título do Artigo"

🐳 Execução via Docker

1. Construa a imagem:
   ```bash
   make docker-build

3. Execute o upload:
   ```bash
   make docker-run-upload

5. Busque por ID:
   ```bash
   make docker-run-seek1 ID=230
   make docker-run-findrec ID=230

7. Busque por título:
   ```bash
   make docker-run-seek2 TITLE="Título do Artigo"

🧼 Limpeza
   Remove os binários gerados:
   make clean

🛠️ Corrigindo erro de indentação no Makefile
   Se ao rodar make aparecer o erro: Makefile:xx: *** missing separator.  Stop.
   Use este comando para corrigir automaticamente as linhas que deveriam começar com TAB:
   ```bash
   perl -pe 's/^(    )/\t/' Makefile > Makefile.fix && mv Makefile.fix Makefile




