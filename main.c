/*
 * Sistema Auto Center Horizonte v1.0
 * PIM - ADS (UNIP) - Digitalizacao de processos de atendimento
 *
 * Modo console, persistencia em arquivos de texto:
 *   clientes.txt -> cpf;nome;telefone
 *   veiculos.txt -> placa;marca;modelo;ano;motor;combustivel;cpf_cliente
 *
 * As funcoes seguem os nomes do pseudocodigo do Capitulo 5 (secao 5.3).
 * Compilar: gcc -Wall -o auto_center main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CLIENTES 500
#define MAX_VEICULOS 1000
#define ARQ_CLIENTES "clientes.txt"
#define ARQ_VEICULOS "veiculos.txt"

/* A. Estruturas de dados */
typedef struct {
    char cpf[12];          /* 11 digitos, identificador unico (RN02) */
    char nome[60];
    char telefone[16];
} Cliente;

typedef struct {
    char placa[8];         /* identificador unico (RN03) */
    char marca[30];
    char modelo[30];
    int  ano;
    char motor[20];
    char combustivel[20];
    char cpf_cliente[12];  /* vinculo com o proprietario (RN01) */
} Veiculo;

static Cliente clientes[MAX_CLIENTES];
static Veiculo veiculos[MAX_VEICULOS];
static int totalClientes = 0;
static int totalVeiculos = 0;
static char opcoesMenu[5][30];

/* ---------- utilidades de entrada ---------- */

static void copiar(char *dest, const char *orig, int tam) {
    strncpy(dest, orig, (size_t)tam - 1);
    dest[tam - 1] = '\0';
}

static void lerTexto(const char *msg, char *dest, int tam) {
    char *p;
    printf("%s", msg);
    if (fgets(dest, tam, stdin) == NULL) { dest[0] = '\0'; return; }
    if (strchr(dest, '\n') == NULL) {            /* descarta o excesso da linha */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
    }
    dest[strcspn(dest, "\r\n")] = '\0';
    while ((p = strchr(dest, ';')) != NULL) *p = ',';   /* ';' separa os campos */
}

static int lerInteiro(const char *msg) {
    char buf[32];
    lerTexto(msg, buf, sizeof buf);
    return atoi(buf);
}

static void soDigitos(const char *orig, char *dest, int tam) {
    int i, j = 0;
    for (i = 0; orig[i] != '\0' && j < tam - 1; i++)
        if (isdigit((unsigned char)orig[i])) dest[j++] = orig[i];
    dest[j] = '\0';
}

static void maiusculas(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

static int confirmar(const char *msg) {
    char r[8];
    lerTexto(msg, r, sizeof r);
    return r[0] == 'S' || r[0] == 's';
}

/* ---------- C. Persistencia em arquivos ---------- */

static int separar(char *linha, char *campos[], int n) {
    int k = 0;
    char *p = linha;
    while (k < n) {
        campos[k++] = p;
        p = strchr(p, ';');
        if (p == NULL) break;
        *p++ = '\0';
    }
    return k == n;
}

static void carregarDados(void) {
    FILE *f;
    char linha[300], *c[7];

    totalClientes = 0;
    totalVeiculos = 0;

    f = fopen(ARQ_CLIENTES, "r");
    if (f != NULL) {
        while (totalClientes < MAX_CLIENTES && fgets(linha, sizeof linha, f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            if (separar(linha, c, 3)) {
                Cliente *x = &clientes[totalClientes++];
                copiar(x->cpf, c[0], sizeof x->cpf);
                copiar(x->nome, c[1], sizeof x->nome);
                copiar(x->telefone, c[2], sizeof x->telefone);
            }
        }
        fclose(f);
    }

    f = fopen(ARQ_VEICULOS, "r");
    if (f != NULL) {
        while (totalVeiculos < MAX_VEICULOS && fgets(linha, sizeof linha, f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            if (separar(linha, c, 7)) {
                Veiculo *v = &veiculos[totalVeiculos++];
                copiar(v->placa, c[0], sizeof v->placa);
                copiar(v->marca, c[1], sizeof v->marca);
                copiar(v->modelo, c[2], sizeof v->modelo);
                v->ano = atoi(c[3]);
                copiar(v->motor, c[4], sizeof v->motor);
                copiar(v->combustivel, c[5], sizeof v->combustivel);
                copiar(v->cpf_cliente, c[6], sizeof v->cpf_cliente);
            }
        }
        fclose(f);
    }
}

static void gravarDados(void) {
    FILE *f;
    int i;

    f = fopen(ARQ_CLIENTES, "w");
    if (f == NULL) { printf("Erro ao gravar %s\n", ARQ_CLIENTES); return; }
    for (i = 0; i < totalClientes; i++)
        fprintf(f, "%s;%s;%s\n", clientes[i].cpf, clientes[i].nome, clientes[i].telefone);
    fclose(f);

    f = fopen(ARQ_VEICULOS, "w");
    if (f == NULL) { printf("Erro ao gravar %s\n", ARQ_VEICULOS); return; }
    for (i = 0; i < totalVeiculos; i++)
        fprintf(f, "%s;%s;%s;%d;%s;%s;%s\n", veiculos[i].placa, veiculos[i].marca,
                veiculos[i].modelo, veiculos[i].ano, veiculos[i].motor,
                veiculos[i].combustivel, veiculos[i].cpf_cliente);
    fclose(f);
}

/* ---------- D. Validacoes ---------- */

static int validarCpf(const char *cpf) {
    int i, k, n, soma, resto, iguais = 1;

    if (strlen(cpf) != 11) return 0;
    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) return 0;
        if (cpf[i] != cpf[0]) iguais = 0;
    }
    if (iguais) return 0;

    for (k = 1; k <= 2; k++) {          /* 1o e 2o digitos verificadores */
        n = 8 + k;                      /* 9 digitos no 1o calculo, 10 no 2o */
        soma = 0;
        for (i = 0; i < n; i++) soma += (cpf[i] - '0') * (n + 1 - i);
        resto = (soma * 10) % 11;
        if (resto == 10) resto = 0;
        if (resto != cpf[n] - '0') return 0;
    }
    return 1;
}

/* Aceita AAA9999 (antiga) e AAA9A99 (Mercosul) */
static int validarPlaca(const char *p) {
    int i;
    if (strlen(p) != 7) return 0;
    for (i = 0; i < 3; i++)
        if (!isalpha((unsigned char)p[i])) return 0;
    if (!isdigit((unsigned char)p[3])) return 0;
    if (!isdigit((unsigned char)p[4]) && !isalpha((unsigned char)p[4])) return 0;
    return isdigit((unsigned char)p[5]) && isdigit((unsigned char)p[6]);
}

/* ---------- E. Funcoes de busca ---------- */

static int buscarClientePorCpf(const char *cpf) {
    int i;
    for (i = 0; i < totalClientes; i++)
        if (strcmp(clientes[i].cpf, cpf) == 0) return i;
    return -1;
}

static int buscarVeiculoPorPlaca(const char *placa) {
    int i;
    for (i = 0; i < totalVeiculos; i++)
        if (strcmp(veiculos[i].placa, placa) == 0) return i;
    return -1;
}

/* O telefone pode ser compartilhado: lista os clientes e o atendente escolhe */
static int escolherClientePorTelefone(const char *telefone) {
    int ocorrencias[MAX_CLIENTES];
    int i, encontrados = 0, escolha;

    for (i = 0; i < totalClientes; i++) {
        if (strcmp(clientes[i].telefone, telefone) == 0) {
            ocorrencias[encontrados++] = i;
            printf("%d. %s\n", encontrados, clientes[i].nome);
        }
    }
    if (encontrados == 0) return -1;
    if (encontrados == 1) return ocorrencias[0];

    printf("Mais de um cliente com este telefone.\n");
    escolha = lerInteiro("Confirme o nome com o cliente e informe o numero: ");
    if (escolha < 1 || escolha > encontrados) return -1;
    return ocorrencias[escolha - 1];
}

/* ---------- F. Cadastro de cliente ---------- */

static void cadastrarCliente(void) {
    char buf[64], cpf[12], nome[60], telefone[16];

    printf("=== CADASTRO DE CLIENTE ===\n");
    lerTexto("CPF (somente numeros): ", buf, sizeof buf);
    soDigitos(buf, cpf, sizeof cpf);
    if (!validarCpf(cpf)) { printf("Erro: CPF invalido!\n"); return; }
    if (buscarClientePorCpf(cpf) != -1) {
        printf("Erro (RN02): CPF ja cadastrado!\n");
        return;
    }
    if (totalClientes == MAX_CLIENTES) { printf("Erro: limite de clientes atingido!\n"); return; }

    do {
        lerTexto("Nome: ", nome, sizeof nome);
        lerTexto("Telefone (DDD + numero): ", buf, sizeof buf);
        soDigitos(buf, telefone, sizeof telefone);
        if (nome[0] == '\0' || telefone[0] == '\0')
            printf("Erro (RN04): nome e telefone sao obrigatorios!\n");
    } while (nome[0] == '\0' || telefone[0] == '\0');

    copiar(clientes[totalClientes].cpf, cpf, sizeof clientes[0].cpf);
    copiar(clientes[totalClientes].nome, nome, sizeof clientes[0].nome);
    copiar(clientes[totalClientes].telefone, telefone, sizeof clientes[0].telefone);
    totalClientes++;
    gravarDados();
    printf("Cliente cadastrado com sucesso!\n");
}

/* ---------- G. Cadastro de veiculo ---------- */

static void cadastrarVeiculo(void) {
    char buf[64], cpf[12], placa[16];
    int indiceCliente;
    Veiculo *v;

    printf("=== CADASTRO DE VEICULO ===\n");
    lerTexto("CPF do proprietario: ", buf, sizeof buf);
    soDigitos(buf, cpf, sizeof cpf);
    indiceCliente = buscarClientePorCpf(cpf);
    if (indiceCliente == -1) {
        printf("Erro (RN01): veiculo nao pode ser cadastrado\nsem um cliente valido!\n");
        return;
    }
    printf("Proprietario: %s\n", clientes[indiceCliente].nome);

    lerTexto("Placa: ", placa, sizeof placa);
    maiusculas(placa);
    if (!validarPlaca(placa)) { printf("Erro: placa invalida!\n"); return; }
    if (buscarVeiculoPorPlaca(placa) != -1) { printf("Erro (RN03): placa ja cadastrada!\n"); return; }
    if (totalVeiculos == MAX_VEICULOS) { printf("Erro: limite de veiculos atingido!\n"); return; }

    v = &veiculos[totalVeiculos];
    copiar(v->placa, placa, sizeof v->placa);
    lerTexto("Marca: ", v->marca, sizeof v->marca);
    lerTexto("Modelo: ", v->modelo, sizeof v->modelo);
    v->ano = lerInteiro("Ano: ");
    lerTexto("Motor: ", v->motor, sizeof v->motor);
    lerTexto("Combustivel: ", v->combustivel, sizeof v->combustivel);
    copiar(v->cpf_cliente, cpf, sizeof v->cpf_cliente);
    totalVeiculos++;
    gravarDados();
    printf("Veiculo vinculado e cadastrado com sucesso!\n");
}

/* ---------- H. Consulta ---------- */

static void exibirCliente(int i) {
    int j, quantidade = 0;
    printf("Cliente encontrado:\n");
    printf("%s | %s | %s\n", clientes[i].cpf, clientes[i].nome, clientes[i].telefone);
    printf("Veiculos vinculados:\n");
    for (j = 0; j < totalVeiculos; j++) {
        if (strcmp(veiculos[j].cpf_cliente, clientes[i].cpf) == 0) {
            printf("%s | %s | %s | %d\n", veiculos[j].placa, veiculos[j].marca,
                   veiculos[j].modelo, veiculos[j].ano);
            quantidade++;
        }
    }
    if (quantidade == 0) printf("Nenhum veiculo vinculado.\n");
}

/* ---------- J (auxiliar). Remocao de registro do vetor ---------- */

static void removerVeiculo(int j) {
    int k;
    for (k = j; k < totalVeiculos - 1; k++) veiculos[k] = veiculos[k + 1];
    totalVeiculos--;
}

/* ---------- I. Atualizacao ---------- */

static int escolherVeiculoDoCliente(int i) {
    char placa[16];
    int j;
    lerTexto("Placa: ", placa, sizeof placa);
    maiusculas(placa);
    j = buscarVeiculoPorPlaca(placa);
    if (j == -1 || strcmp(veiculos[j].cpf_cliente, clientes[i].cpf) != 0) {
        printf("Veiculo nao encontrado para este cliente.\n");
        return -1;
    }
    return j;
}

static void atualizarCliente(int i) {
    char novoNome[60], buf[64], novoTelefone[16];

    printf("Deixe em branco para manter o valor atual.\n");
    lerTexto("Novo nome: ", novoNome, sizeof novoNome);
    lerTexto("Novo telefone: ", buf, sizeof buf);
    soDigitos(buf, novoTelefone, sizeof novoTelefone);

    if (novoNome[0] != '\0') copiar(clientes[i].nome, novoNome, sizeof clientes[i].nome);
    if (novoTelefone[0] != '\0') copiar(clientes[i].telefone, novoTelefone, sizeof clientes[i].telefone);
    gravarDados();
    printf("Cadastro atualizado com sucesso!\n");
}

static void atualizarVeiculo(int j) {
    char marca[30], modelo[30], motor[20], combustivel[20];
    int ano;

    printf("Deixe em branco para manter o valor atual.\n");
    lerTexto("Nova marca: ", marca, sizeof marca);
    lerTexto("Novo modelo: ", modelo, sizeof modelo);
    ano = lerInteiro("Novo ano (0 mantem): ");
    lerTexto("Novo motor: ", motor, sizeof motor);
    lerTexto("Novo combustivel: ", combustivel, sizeof combustivel);

    if (marca[0] != '\0') copiar(veiculos[j].marca, marca, sizeof veiculos[j].marca);
    if (modelo[0] != '\0') copiar(veiculos[j].modelo, modelo, sizeof veiculos[j].modelo);
    if (ano > 0) veiculos[j].ano = ano;
    if (motor[0] != '\0') copiar(veiculos[j].motor, motor, sizeof veiculos[j].motor);
    if (combustivel[0] != '\0') copiar(veiculos[j].combustivel, combustivel, sizeof veiculos[j].combustivel);
    gravarDados();
    printf("Cadastro atualizado com sucesso!\n");
}

/* ---------- J. Exclusao ---------- */

static void excluirCliente(int i) {
    char cpf[12];
    int j, k, quantidade = 0;

    copiar(cpf, clientes[i].cpf, sizeof cpf);
    for (j = 0; j < totalVeiculos; j++)
        if (strcmp(veiculos[j].cpf_cliente, cpf) == 0) quantidade++;

    printf("Excluir %s e %d veiculo(s) vinculado(s)?\n", clientes[i].nome, quantidade);
    if (!confirmar("Confirma (S/N)? ")) { printf("Exclusao cancelada.\n"); return; }

    j = 0;
    while (j < totalVeiculos) {
        if (strcmp(veiculos[j].cpf_cliente, cpf) == 0)
            removerVeiculo(j);              /* exclusao em cascata (RN05) */
        else
            j++;
    }
    for (k = i; k < totalClientes - 1; k++) clientes[k] = clientes[k + 1];
    totalClientes--;
    gravarDados();
    printf("Cliente e veiculos vinculados excluidos com sucesso!\n");
}

static void excluirVeiculo(int j) {
    printf("Excluir o veiculo %s?\n", veiculos[j].placa);
    if (!confirmar("Confirma (S/N)? ")) { printf("Exclusao cancelada.\n"); return; }
    removerVeiculo(j);
    gravarDados();
    printf("Veiculo excluido com sucesso!\n");
}

/* ---------- I. Acoes sobre o cadastro localizado ---------- */

static void acaoSobreCadastro(int i) {
    int acao, j;

    printf("1 - Atualizar cliente   2 - Atualizar veiculo\n");
    printf("3 - Excluir cliente     4 - Excluir veiculo\n");
    printf("0 - Voltar\n");
    acao = lerInteiro("Opcao: ");

    switch (acao) {
    case 1: atualizarCliente(i); break;
    case 2:
        j = escolherVeiculoDoCliente(i);
        if (j != -1) atualizarVeiculo(j);
        break;
    case 3: excluirCliente(i); break;
    case 4:
        j = escolherVeiculoDoCliente(i);
        if (j != -1) excluirVeiculo(j);
        break;
    case 0: return;
    default: printf("Opcao invalida!\n");
    }
}

static void consultarCadastro(void) {
    char buf[64], chave[16];
    int criterio, indiceCliente = -1, indiceVeiculo;

    printf("=== CONSULTA DE CADASTRO ===\n");
    printf("1 - Buscar por CPF\n2 - Buscar por telefone\n3 - Buscar por placa\n");
    criterio = lerInteiro("Opcao: ");

    switch (criterio) {
    case 1:
        lerTexto("CPF: ", buf, sizeof buf);
        soDigitos(buf, chave, sizeof chave);
        indiceCliente = buscarClientePorCpf(chave);
        break;
    case 2:
        lerTexto("Telefone: ", buf, sizeof buf);
        soDigitos(buf, chave, sizeof chave);
        indiceCliente = escolherClientePorTelefone(chave);
        break;
    case 3:
        lerTexto("Placa: ", chave, sizeof chave);
        maiusculas(chave);
        indiceVeiculo = buscarVeiculoPorPlaca(chave);
        if (indiceVeiculo != -1)
            indiceCliente = buscarClientePorCpf(veiculos[indiceVeiculo].cpf_cliente);
        break;
    default:
        printf("Opcao invalida!\n");
        return;
    }

    if (indiceCliente == -1) { printf("Nenhum cadastro encontrado.\n"); return; }
    exibirCliente(indiceCliente);
    acaoSobreCadastro(indiceCliente);
}

/* ---------- K. Relatorios ---------- */

static void emitirRelatorios(void) {
    int i;

    printf("=== RELATORIO DE CLIENTES ===\n");
    if (totalClientes == 0) {
        printf("Nenhum cliente cadastrado.\n");
    } else {
        printf("CPF | NOME | TELEFONE\n");
        for (i = 0; i < totalClientes; i++)
            printf("%s | %s | %s\n", clientes[i].cpf, clientes[i].nome, clientes[i].telefone);
    }
    printf("Total de clientes: %d\n", totalClientes);

    printf("=== RELATORIO DE VEICULOS ===\n");
    if (totalVeiculos == 0) {
        printf("Nenhum veiculo cadastrado.\n");
    } else {
        printf("PLACA | MARCA | MODELO | ANO | CPF DO PROPRIETARIO\n");
        for (i = 0; i < totalVeiculos; i++)
            printf("%s | %s | %s | %d | %s\n", veiculos[i].placa, veiculos[i].marca,
                   veiculos[i].modelo, veiculos[i].ano, veiculos[i].cpf_cliente);
    }
    printf("Total de veiculos: %d\n", totalVeiculos);
}

/* ---------- B. Programa principal ---------- */

int main(void) {
    int opcao = -1, i;

    carregarDados();
    strcpy(opcoesMenu[0], "1 - Cadastrar cliente");
    strcpy(opcoesMenu[1], "2 - Cadastrar veiculo");
    strcpy(opcoesMenu[2], "3 - Consultar cadastro");
    strcpy(opcoesMenu[3], "4 - Emitir relatorios");
    strcpy(opcoesMenu[4], "0 - Sair");

    while (opcao != 0) {
        printf("\n=== AUTO CENTER HORIZONTE ===\n");
        for (i = 0; i < 5; i++) printf("%s\n", opcoesMenu[i]);
        opcao = lerInteiro("Escolha: ");
        printf("\n");

        switch (opcao) {
        case 1: cadastrarCliente();  break;
        case 2: cadastrarVeiculo();  break;
        case 3: consultarCadastro(); break;
        case 4: emitirRelatorios();  break;
        case 0: printf("Encerrando o sistema...\n"); break;
        default: printf("Opcao invalida!\n");
        }
    }
    return 0;
}
