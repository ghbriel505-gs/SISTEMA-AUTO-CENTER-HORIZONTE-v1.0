# Sistema Auto Center Horizonte v1.0

Projeto lógico do sistema de digitalização de processos de atendimento do Auto Center Horizonte,
desenvolvido como PIM do curso de Análise e Desenvolvimento de Sistemas (UNIP).

## Escopo
Este trabalho é um **projeto lógico e conceitual**: a especificação (Capítulo 3) e os algoritmos em
pseudocódigo e fluxogramas (Capítulo 5) são os produtos principais.
O arquivo `main.c` é um **protótipo de referência** que traduz o pseudocódigo para C, usado para
validar a lógica. Ele não constitui uma implementação final do sistema.

O pré-cadastro (RN06, RN07 e RF07) está especificado, mas fica fora do protótipo.
Sua implementação está prevista para a versão 2.0.

## O que o protótipo cobre
- Cadastro de clientes e de veículos, com validação de CPF e de placa (padrão antigo e Mercosul)
- Consulta por CPF, telefone ou placa
- Atualização e exclusão de cadastros, a partir da consulta
- Exclusão de cliente com remoção em cascata dos veículos, com confirmação (RN05)
- Relatórios de clientes e de veículos

## Tecnologias
- Linguagem C, modo console
- Persistência em arquivos de texto (`clientes.txt` e `veiculos.txt`)

## Como compilar e executar o protótipo
```
gcc -Wall -o auto_center main.c
./auto_center        (no Windows: auto_center.exe)
```

## Arquivos de dados
Criados automaticamente no primeiro cadastro, na pasta de execução.
Formato (campos separados por `;`):
- `clientes.txt`: `cpf;nome;telefone`
- `veiculos.txt`: `placa;marca;modelo;ano;motor;combustivel;cpf_cliente`

Os arquivos `clientes_exemplo.txt` e `veiculos_exemplo.txt` têm dados fictícios.
Para testar, copie-os com os nomes `clientes.txt` e `veiculos.txt`.
Este repositório não contém dados reais de clientes.

## Metodologia
Ciclo de vida em Cascata; fluxo da equipe com Kanban (Trello).
