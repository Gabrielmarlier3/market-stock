# todo: deixar em inglês e mais bonitinho

Para rodar usa o 
```shell
DB_PATH=/home/gabriel/repositorios/loja_do_marcos/db.txt ./main
```


Bibliografia


https://www.ibm.com/docs/en/i/7.4.0
https://cppreference.com/
https://www.geeksforgeeks.org/c/clear-console-c-language/
https://www.programiz.com/dsa/queue

Livros 

K. N. King - C Programming


Lojinha fo seu marcos:

Você vai desenvolver o sistema do estoque de uma loja. Na interface do sistema por terminal o usuario deverá poder escolher entre as opções de:
- adicionar item
- deletar item
- editar item
- buscar item
- sair

Todo item possui as propriedades de:
- ID
- Nome
- Quantidade
- Preço

Para criar um item voce coloca o nome do item, a quantidade, e o preço. Ao fazer isso caso não exista no banco de dados um item com este nome um id será gerado para o item e ele deve ser armazenado no banco de dados.

Para deletar o item o usuario deverá enviar o nome ou id do item e caso o item exista no banco de dados ele deverá ser deletado

Para buscar o item o usuario deverá escolher se quer ver todos os itens ou se quer buscar um item específico. Caso o usuario queira ver um item específico ele deverá inserir o id ou nome do item e se este existir deverá ter todas suas informações mostradas

Para editar um item o usuario deverá buscar o item inserindo nome ou id e caso o item exista ele poderá escolher qual informação do item que ele quer alterar

Observações:
- Não pode sobre nenhuma circunstância existir 2 itens com o mesmo nome no banco
- Se um item tiver apenas 3 exemplares ou menos o sistema deve avisar o usuario que o item está acabando
- Caso um item tenha 0 unidades no estoque ele é removido do banco e o usuario é avisado
- Caso o usuario saia do programa em qualquer momento todas as últimos modificações que ele fez devem se manter salvas para a próxima vez que ele entrar no programa
- O unico "banco de dados" que vocês podem usar é um arquivo txt para armazenar as informações
- O caminho para o arquivo txt deve ser especificado em um arquivo de variáveis de ambiente ".env" não podendo estar escrito estático direto no codigo
- O codigo deve ser feito completamente em C puro sem uso de bibliotecas que não forem nativas do C
- Todo e qualquer uso de I.A será punido com a retirada do membro de dentro do projeto
- O programa so deve fechar se o usuario pedir para sair
- O programa deve ser entregue via link de um repositório público do github
- Façam um README bonitinho pra mim pô