📦 TP2 — Instruções & Comandos

⚙️ Compilação e execução local

1. Compile os binários:
   ```bash
   make build

2. Execute o upload com o CSV:
   ./bin/upload ./data/input.csv

3. Busque por ID diretamente no arquivo:
   ./bin/findrec ./data/artigos.dat 230

4.Busque por título (índice secundário):
   ./bin/seek2 "Título do Artigo"

🐳 Execução via Docker

1. Construa a imagem:
   make docker-build

2. Execute o upload:
   make docker-run-upload

3. Busque por ID:
   make docker-run-seek1 ID=230
   make docker-run-findrec ID=230

4. Busque por título:
   make docker-run-seek2 TITLE="Título do Artigo"

🧼 Limpeza
   Remove os binários gerados:
   make clean

🛠️ Corrigindo erro de indentação no Makefile
   Se ao rodar make aparecer o erro: Makefile:xx: *** missing separator.  Stop.
   Use este comando para corrigir automaticamente as linhas que deveriam começar com TAB:
   perl -pe 's/^(    )/\t/' Makefile > Makefile.fix && mv Makefile.fix Makefile




