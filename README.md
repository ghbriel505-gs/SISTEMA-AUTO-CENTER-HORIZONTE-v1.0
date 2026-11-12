# Sistema Auto Center Horizonte v1.0

Sistema de digitalização de processos de atendimento para o Auto Center Horizonte,
desenvolvido como PIM do curso de Análise e Desenvolvimento de Sistemas (UNIP).

## Funcionalidades
- Cadastro, consulta, atualização e exclusão de clientes e veículos
- Busca por CPF, telefone ou placa
- Exclusão de cliente com remoção em cascata dos veículos (com confirmação)
- Validação de CPF e placa (padrão antigo e Mercosul)
- Relatórios de clientes e de veículos

## Tecnologias
- Linguagem C, modo console
- Persistência em arquivos de texto (`clientes.txt` e `veiculos.txt`)

## Como compilar e executar
```
gcc -Wall -o auto_center main.c
./auto_center        (no Windows: auto_center.exe)
```

## Arquivos de dados
Criados automaticamente no primeiro cadastro, na pasta de execução.
Formato (campos separados por `;`):
- `clientes.txt`: `cpf;nome;telefone;endereco`
- `veiculos.txt`: `placa;modelo;ano;combustivel;cpf_proprietario`

Os arquivos `clientes_exemplo.txt` e `veiculos_exemplo.txt` têm dados fictícios.
Para testar, copie-os com os nomes `clientes.txt` e `veiculos.txt`.
Este repositório não contém dados reais de clientes.

## Metodologia
Ciclo de vida em Cascata; fluxo da equipe com Kanban (Trello).
