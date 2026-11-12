# Sistema Auto Center Horizonte v1.0

Sistema de digitalização de processos de atendimento para o Auto Center Horizonte,
desenvolvido como PIM do curso de Análise e Desenvolvimento de Sistemas (UNIP).

## Funcionalidades
- Cadastro, consulta, atualização e exclusão de clientes e veículos
- Busca por CPF, telefone ou placa
- Exclusão de cliente com remoção em cascata dos veículos (com confirmação)
- Relatórios

## Tecnologias
- Linguagem C, modo console
- Persistência em arquivos de texto (`clientes.txt` e `veiculos.txt`)

## Como compilar e executar
gcc -o auto_center main.c
./auto_center

(Ajuste o nome dos arquivos conforme o seu projeto.)

## Observações
- Os arquivos de dados são criados na primeira execução.
- Este repositório não contém dados reais de clientes.

## Metodologia
Ciclo de vida em Cascata; fluxo da equipe com Kanban (Trello).

## Autor
Seu nome – UNIP, ADS
