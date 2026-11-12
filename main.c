/*
 * Sistema Auto Center Horizonte v1.0
 * PIM - ADS (UNIP) - Digitalizacao de processos de atendimento
 *
 * Modo console, persistencia em arquivos de texto:
 *   clientes.txt  -> cpf;nome;telefone;endereco
 *   veiculos.txt  -> placa;modelo;ano;combustivel;cpf_proprietario
 *
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

typedef struct {
    char cpf[12];       /* 11 digitos, identificador unico */
    char nome[60];
    char telefone[16];
    char endereco[80];
} Cliente;

typedef struct {
    char placa[8];      /* 7 caracteres, identificador unico */
    char modelo[40];
    int  ano;
    char combustivel[16];
    char cpf[12];       /* referencia ao proprietario */
} Veiculo;

static Cliente clientes[MAX_CLIENTES];
static Veiculo veiculos[MAX_VEICULOS];
static int totalClientes = 0;
static int totalVeiculos = 0;

static const char *MENU[10] = {
    "1 - Cadastrar cliente",
    "2 - Cadastrar veiculo",
    "3 - Consultar (CPF, telefone ou placa)",
    "4 - Atualizar cliente",
    "5 - Atualizar veiculo",
    "6 - Excluir cliente (e seus veiculos)",
    "7 - Excluir veiculo",
    "8 - Relatorio de clientes",
    "9 - Relatorio de veiculos",
    "0 - Sair"
};

/* ---------- entrada e utilidades ---------- */

static void lerLinha(const char *msg, char *dest, int tam) {
    char *p;
    printf("%s", msg);
    if (fgets(dest, tam, stdin) == NULL) { dest[0] = '\0'; return; }
    dest[strcspn(dest, "\r\n")] = '\0';
    while ((p = strchr(dest, ';')) != NULL) *p = ',';  /* ';' e o separador do arquivo */
}

static int lerInteiro(const char *msg) {
    char buf[32];
    lerLinha(msg, buf, sizeof buf);
    return atoi(buf);
}

static int confirmar(const char *msg) {
    char buf[8];
    lerLinha(msg, buf, sizeof buf);
    return toupper((unsigned char)buf[0]) == 'S';
}

static void soDigitos(const char *orig, char *dest, int tam) {
    int j = 0, i;
    for (i = 0; orig[i] != '\0' && j < tam - 1; i++)
        if (isdigit((unsigned char)orig[i])) dest[j++] = orig[i];
    dest[j] = '\0';
}

static void maiusculas(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

/* ---------- validacoes ---------- */

static int validarCPF(const char *cpf) {
    int i, soma, d1, d2, iguais = 1;
    if (strlen(cpf) != 11) return 0;
    for (i = 0; i < 11; i++) {
        if (!isdigit((unsigned char)cpf[i])) return 0;
        if (cpf[i] != cpf[0]) iguais = 0;
    }
    if (iguais) return 0;
    for (soma = 0, i = 0; i < 9; i++) soma += (cpf[i] - '0') * (10 - i);
    d1 = (soma * 10) % 11; if (d1 == 10) d1 = 0;
    for (soma = 0, i = 0; i < 10; i++) soma += (cpf[i] - '0') * (11 - i);
    d2 = (soma * 10) % 11; if (d2 == 10) d2 = 0;
    return d1 == cpf[9] - '0' && d2 == cpf[10] - '0';
}

/* Aceita AAA9999 (antiga) e AAA9A99 (Mercosul) */
static int validarPlaca(const char *p) {
    int i;
    if (strlen(p) != 7) return 0;
    for (i = 0; i < 3; i++) if (!isupper((unsigned char)p[i])) return 0;
    if (!isdigit((unsigned char)p[3])) return 0;
    if (!isdigit((unsigned char)p[4]) && !isupper((unsigned char)p[4])) return 0;
    return isdigit((unsigned char)p[5]) && isdigit((unsigned char)p[6]);
}

static int validarTelefone(const char *t) {
    int n = (int)strlen(t);
    return n == 10 || n == 11;
}

/* ---------- busca ---------- */

static int buscarClientePorCPF(const char *cpf) {
    int i;
    for (i = 0; i < totalClientes; i++)
        if (strcmp(clientes[i].cpf, cpf) == 0) return i;
    return -1;
}

static int buscarClientePorTelefone(const char *tel) {
    int i;
    for (i = 0; i < totalClientes; i++)
        if (strcmp(clientes[i].telefone, tel) == 0) return i;
    return -1;
}

static int buscarVeiculoPorPlaca(const char *placa) {
    int i;
    for (i = 0; i < totalVeiculos; i++)
        if (strcmp(veiculos[i].placa, placa) == 0) return i;
    return -1;
}

/* ---------- persistencia ---------- */

static void salvarDados(void) {
    FILE *f;
    int i;
    f = fopen(ARQ_CLIENTES, "w");
    if (f == NULL) { printf("Erro ao gravar %s\n", ARQ_CLIENTES); return; }
    for (i = 0; i < totalClientes; i++)
        fprintf(f, "%s;%s;%s;%s\n", clientes[i].cpf, clientes[i].nome,
                clientes[i].telefone, clientes[i].endereco);
    fclose(f);
    f = fopen(ARQ_VEICULOS, "w");
    if (f == NULL) { printf("Erro ao gravar %s\n", ARQ_VEICULOS); return; }
    for (i = 0; i < totalVeiculos; i++)
        fprintf(f, "%s;%s;%d;%s;%s\n", veiculos[i].placa, veiculos[i].modelo,
                veiculos[i].ano, veiculos[i].combustivel, veiculos[i].cpf);
    fclose(f);
}

static void carregarDados(void) {
    FILE *f;
    char linha[256], *campo[5];
    int n;
    char *p;

    f = fopen(ARQ_CLIENTES, "r");
    if (f != NULL) {
        while (totalClientes < MAX_CLIENTES && fgets(linha, sizeof linha, f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            n = 0; p = linha;
            while (n < 4) { campo[n++] = p; p = strchr(p, ';'); if (!p) break; *p++ = '\0'; }
            if (n == 4) {
                Cliente *c = &clientes[totalClientes++];
                strncpy(c->cpf, campo[0], sizeof c->cpf - 1);           c->cpf[sizeof c->cpf - 1] = '\0';
                strncpy(c->nome, campo[1], sizeof c->nome - 1);         c->nome[sizeof c->nome - 1] = '\0';
                strncpy(c->telefone, campo[2], sizeof c->telefone - 1); c->telefone[sizeof c->telefone - 1] = '\0';
                strncpy(c->endereco, campo[3], sizeof c->endereco - 1); c->endereco[sizeof c->endereco - 1] = '\0';
            }
        }
        fclose(f);
    }

    f = fopen(ARQ_VEICULOS, "r");
    if (f != NULL) {
        while (totalVeiculos < MAX_VEICULOS && fgets(linha, sizeof linha, f)) {
            linha[strcspn(linha, "\r\n")] = '\0';
            n = 0; p = linha;
            while (n < 5) { campo[n++] = p; p = strchr(p, ';'); if (!p) break; *p++ = '\0'; }
            if (n == 5) {
                Veiculo *v = &veiculos[totalVeiculos++];
                strncpy(v->placa, campo[0], sizeof v->placa - 1);             v->placa[sizeof v->placa - 1] = '\0';
                strncpy(v->modelo, campo[1], sizeof v->modelo - 1);           v->modelo[sizeof v->modelo - 1] = '\0';
                v->ano = atoi(campo[2]);
                strncpy(v->combustivel, campo[3], sizeof v->combustivel - 1); v->combustivel[sizeof v->combustivel - 1] = '\0';
                strncpy(v->cpf, campo[4], sizeof v->cpf - 1);                 v->cpf[sizeof v->cpf - 1] = '\0';
            }
        }
        fclose(f);
    }
}

/* ---------- exibicao ---------- */

static void mostrarCliente(const Cliente *c) {
    printf("  CPF........: %s\n  Nome.......: %s\n  Telefone...: %s\n  Endereco...: %s\n",
           c->cpf, c->nome, c->telefone, c->endereco);
}

static void mostrarVeiculo(const Veiculo *v) {
    printf("  Placa: %s | Modelo: %s | Ano: %d | Combustivel: %s | CPF: %s\n",
           v->placa, v->modelo, v->ano, v->combustivel, v->cpf);
}

static void mostrarVeiculosDoCliente(const char *cpf) {
    int i, achou = 0;
    for (i = 0; i < totalVeiculos; i++)
        if (strcmp(veiculos[i].cpf, cpf) == 0) { mostrarVeiculo(&veiculos[i]); achou = 1; }
    if (!achou) printf("  (nenhum veiculo cadastrado)\n");
}

/* ---------- funcionalidades ---------- */

static void cadastrarCliente(void) {
    Cliente c;
    char buf[64];
    if (totalClientes >= MAX_CLIENTES) { printf("Limite de clientes atingido.\n"); return; }

    lerLinha("CPF (somente numeros): ", buf, sizeof buf);
    soDigitos(buf, c.cpf, sizeof c.cpf);
    if (!validarCPF(c.cpf)) { printf("CPF invalido.\n"); return; }
    if (buscarClientePorCPF(c.cpf) >= 0) { printf("CPF ja cadastrado.\n"); return; }

    lerLinha("Nome: ", c.nome, sizeof c.nome);
    if (strlen(c.nome) == 0) { printf("Nome obrigatorio.\n"); return; }

    lerLinha("Telefone (DDD + numero): ", buf, sizeof buf);
    soDigitos(buf, c.telefone, sizeof c.telefone);
    if (!validarTelefone(c.telefone)) { printf("Telefone invalido (use 10 ou 11 digitos).\n"); return; }
    if (buscarClientePorTelefone(c.telefone) >= 0) { printf("Telefone ja cadastrado.\n"); return; }

    lerLinha("Endereco: ", c.endereco, sizeof c.endereco);

    clientes[totalClientes++] = c;
    salvarDados();
    printf("Cliente cadastrado com sucesso.\n");
}

static void cadastrarVeiculo(void) {
    Veiculo v;
    char buf[64];
    if (totalVeiculos >= MAX_VEICULOS) { printf("Limite de veiculos atingido.\n"); return; }

    lerLinha("CPF do proprietario: ", buf, sizeof buf);
    soDigitos(buf, v.cpf, sizeof v.cpf);
    if (buscarClientePorCPF(v.cpf) < 0) { printf("Cliente nao encontrado. Cadastre o cliente primeiro.\n"); return; }

    lerLinha("Placa (ex.: ABC1234 ou ABC1D23): ", v.placa, sizeof v.placa);
    maiusculas(v.placa);
    if (!validarPlaca(v.placa)) { printf("Placa invalida.\n"); return; }
    if (buscarVeiculoPorPlaca(v.placa) >= 0) { printf("Placa ja cadastrada.\n"); return; }

    lerLinha("Modelo: ", v.modelo, sizeof v.modelo);
    if (strlen(v.modelo) == 0) { printf("Modelo obrigatorio.\n"); return; }
    v.ano = lerInteiro("Ano: ");
    if (v.ano < 1900 || v.ano > 2100) { printf("Ano invalido.\n"); return; }
    lerLinha("Combustivel: ", v.combustivel, sizeof v.combustivel);

    veiculos[totalVeiculos++] = v;
    salvarDados();
    printf("Veiculo cadastrado com sucesso.\n");
}

static void consultar(void) {
    char buf[64], chave[16];
    int idx, opcao;

    printf("Buscar por: 1-CPF  2-Telefone  3-Placa\n");
    opcao = lerInteiro("Opcao: ");

    switch (opcao) {
    case 1:
        lerLinha("CPF: ", buf, sizeof buf);
        soDigitos(buf, chave, sizeof chave);
        idx = buscarClientePorCPF(chave);
        break;
    case 2:
        lerLinha("Telefone: ", buf, sizeof buf);
        soDigitos(buf, chave, sizeof chave);
        idx = buscarClientePorTelefone(chave);
        break;
    case 3:
        lerLinha("Placa: ", chave, sizeof chave);
        maiusculas(chave);
        idx = buscarVeiculoPorPlaca(chave);
        if (idx < 0) { printf("Nao encontrado.\n"); return; }
        idx = buscarClientePorCPF(veiculos[idx].cpf);
        break;
    default:
        printf("Opcao invalida.\n");
        return;
    }

    if (idx < 0) { printf("Nao encontrado.\n"); return; }
    printf("\n--- Cliente ---\n");
    mostrarCliente(&clientes[idx]);
    printf("--- Veiculos ---\n");
    mostrarVeiculosDoCliente(clientes[idx].cpf);
}

static void atualizarCliente(void) {
    char buf[64], novo[80];
    int idx;
    lerLinha("CPF do cliente: ", buf, sizeof buf);
    soDigitos(buf, buf, sizeof buf);
    idx = buscarClientePorCPF(buf);
    if (idx < 0) { printf("Cliente nao encontrado.\n"); return; }

    mostrarCliente(&clientes[idx]);
    printf("(Deixe em branco para manter o valor atual. O CPF nao pode ser alterado.)\n");

    lerLinha("Novo nome: ", novo, sizeof novo);
    if (strlen(novo) > 0) { strncpy(clientes[idx].nome, novo, sizeof clientes[idx].nome - 1); }

    lerLinha("Novo telefone: ", buf, sizeof buf);
    if (strlen(buf) > 0) {
        soDigitos(buf, novo, sizeof novo);
        int outro = buscarClientePorTelefone(novo);
        if (!validarTelefone(novo)) printf("Telefone invalido, mantido o anterior.\n");
        else if (outro >= 0 && outro != idx) printf("Telefone ja usado por outro cliente, mantido o anterior.\n");
        else strncpy(clientes[idx].telefone, novo, sizeof clientes[idx].telefone - 1);
    }

    lerLinha("Novo endereco: ", novo, sizeof novo);
    if (strlen(novo) > 0) strncpy(clientes[idx].endereco, novo, sizeof clientes[idx].endereco - 1);

    salvarDados();
    printf("Cliente atualizado.\n");
}

static void atualizarVeiculo(void) {
    char placa[16], novo[64];
    int idx, ano;
    lerLinha("Placa do veiculo: ", placa, sizeof placa);
    maiusculas(placa);
    idx = buscarVeiculoPorPlaca(placa);
    if (idx < 0) { printf("Veiculo nao encontrado.\n"); return; }

    mostrarVeiculo(&veiculos[idx]);
    printf("(Deixe em branco para manter o valor atual. Placa e proprietario nao podem ser alterados.)\n");

    lerLinha("Novo modelo: ", novo, sizeof novo);
    if (strlen(novo) > 0) strncpy(veiculos[idx].modelo, novo, sizeof veiculos[idx].modelo - 1);

    lerLinha("Novo ano: ", novo, sizeof novo);
    if (strlen(novo) > 0) {
        ano = atoi(novo);
        if (ano >= 1900 && ano <= 2100) veiculos[idx].ano = ano;
        else printf("Ano invalido, mantido o anterior.\n");
    }

    lerLinha("Novo combustivel: ", novo, sizeof novo);
    if (strlen(novo) > 0) strncpy(veiculos[idx].combustivel, novo, sizeof veiculos[idx].combustivel - 1);

    salvarDados();
    printf("Veiculo atualizado.\n");
}

static void excluirCliente(void) {
    char buf[64];
    int idx, i, qtd = 0;
    lerLinha("CPF do cliente a excluir: ", buf, sizeof buf);
    soDigitos(buf, buf, sizeof buf);
    idx = buscarClientePorCPF(buf);
    if (idx < 0) { printf("Cliente nao encontrado.\n"); return; }

    mostrarCliente(&clientes[idx]);
    for (i = 0; i < totalVeiculos; i++)
        if (strcmp(veiculos[i].cpf, clientes[idx].cpf) == 0) qtd++;
    printf("Atencao: %d veiculo(s) deste cliente tambem sera(ao) excluido(s).\n", qtd);

    if (!confirmar("Confirma a exclusao? (S/N): ")) { printf("Operacao cancelada.\n"); return; }

    /* exclusao em cascata: remove os veiculos do cliente */
    for (i = totalVeiculos - 1; i >= 0; i--) {
        if (strcmp(veiculos[i].cpf, clientes[idx].cpf) == 0) {
            veiculos[i] = veiculos[totalVeiculos - 1];
            totalVeiculos--;
        }
    }
    clientes[idx] = clientes[totalClientes - 1];
    totalClientes--;

    salvarDados();
    printf("Cliente e veiculos excluidos.\n");
}

static void excluirVeiculo(void) {
    char placa[16];
    int idx;
    lerLinha("Placa do veiculo a excluir: ", placa, sizeof placa);
    maiusculas(placa);
    idx = buscarVeiculoPorPlaca(placa);
    if (idx < 0) { printf("Veiculo nao encontrado.\n"); return; }

    mostrarVeiculo(&veiculos[idx]);
    if (!confirmar("Confirma a exclusao? (S/N): ")) { printf("Operacao cancelada.\n"); return; }

    veiculos[idx] = veiculos[totalVeiculos - 1];
    totalVeiculos--;
    salvarDados();
    printf("Veiculo excluido.\n");
}

static void relatorioClientes(void) {
    int i, j, qtd;
    printf("\n=== RELATORIO DE CLIENTES (%d) ===\n", totalClientes);
    for (i = 0; i < totalClientes; i++) {
        qtd = 0;
        for (j = 0; j < totalVeiculos; j++)
            if (strcmp(veiculos[j].cpf, clientes[i].cpf) == 0) qtd++;
        printf("%s | %s | %s | veiculos: %d\n",
               clientes[i].cpf, clientes[i].nome, clientes[i].telefone, qtd);
    }
}

static void relatorioVeiculos(void) {
    int i, c;
    printf("\n=== RELATORIO DE VEICULOS (%d) ===\n", totalVeiculos);
    for (i = 0; i < totalVeiculos; i++) {
        c = buscarClientePorCPF(veiculos[i].cpf);
        printf("%s | %s | %d | %s | proprietario: %s\n",
               veiculos[i].placa, veiculos[i].modelo, veiculos[i].ano,
               veiculos[i].combustivel, c >= 0 ? clientes[c].nome : "(desconhecido)");
    }
}

/* ---------- programa principal ---------- */

int main(void) {
    int opcao, i;

    carregarDados();

    do {
        printf("\n===== AUTO CENTER HORIZONTE =====\n");
        for (i = 0; i < 10; i++) printf("%s\n", MENU[i]);
        opcao = lerInteiro("Escolha: ");
        printf("\n");

        switch (opcao) {
        case 1: cadastrarCliente();   break;
        case 2: cadastrarVeiculo();   break;
        case 3: consultar();          break;
        case 4: atualizarCliente();   break;
        case 5: atualizarVeiculo();   break;
        case 6: excluirCliente();     break;
        case 7: excluirVeiculo();     break;
        case 8: relatorioClientes();  break;
        case 9: relatorioVeiculos();  break;
        case 0: printf("Encerrando. Dados salvos.\n"); break;
        default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    return 0;
}
