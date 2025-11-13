#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif

#define LINHAS 8
#define COLUNAS 12
#define PRECO_INTEIRA 40.00
#define PRECO_MEIA 20.00

typedef struct {
    char fileira;
    int coluna;
    int ocupada;
    int meia;
    int pcd;
} Poltrona;

Poltrona sala[LINHAS][COLUNAS];
int total_clientes = 0;
int clientes_inteira = 0;
int clientes_meia = 0;
int clientes_pcd = 0;
float receita_total = 0.0;

// === CORES ===
#ifdef _WIN32
    #define VERDE   10
    #define VERMELHO 12
    #define AMARELO 14
    #define CIANO   11
    #define BRANCO  15
    #define CINZA   8
#else
    #define VERDE   "\033[32m"
    #define VERMELHO "\033[31m"
    #define AMARELO "\033[33m"
    #define CIANO   "\033[36m"
    #define BRANCO  "\033[37m"
    #define CINZA   "\033[90m"
    #define RESET   "\033[0m"
#endif

// === FUNÇÕES PORTÁVEIS ===
void gotoxy(int x, int y);
void textcolor(const char* cor);
void limpa_tela();
int getch_portable();
void sleep_ms(int ms);
void limpar_buffer();

// === FUNÇÕES DO SISTEMA ===
void inicializa_sala();
void desenha_tela_principal();
void desenha_mapa_poltronas(int hl, int hc);
void reservar_poltrona();
void cancelar_reserva();
void relatorio_simples();
void relatorio_detalhado();
void grafico_barras();
char menu_principal();
int valida_poltrona(char fileira, int coluna);
void centraliza_texto(char *texto, int y);
void tela_cinema();

int main() {
#ifdef _WIN32
    SetConsoleTitle("CINEMA ESTRELA CADENTE - Gestão de Reservas");
#else
    printf("\033[?25l"); // Esconde cursor no Linux
#endif

    inicializa_sala();
    char opcao;

    do {
        desenha_tela_principal();
        opcao = menu_principal();

        switch(opcao) {
            case '1': reservar_poltrona(); break;
            case '2': cancelar_reserva(); break;
            case '3': relatorio_simples(); break;
            case '4': relatorio_detalhado(); break;
            case '5': grafico_barras(); break;
            case '6':
                limpa_tela();
                centraliza_texto("Obrigado por usar o sistema!", 12);
                centraliza_texto("CINEMA ESTRELA CADENTE", 14);
                gotoxy(30, 16); printf("Pressione qualquer tecla para sair...");
                getch_portable();
#ifdef __linux__
                printf("\033[?25h" RESET); // Mostra cursor
#endif
                return 0;
            default:
                limpa_tela();
                centraliza_texto("Opção inválida! Tente novamente.", 12);
                sleep_ms(1500);
        }
    } while(1);

    return 0;
}

// === IMPLEMENTAÇÃO DAS FUNÇÕES PORTÁVEIS ===

#ifdef _WIN32
void gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void textcolor(const char* cor) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)atoi(cor));
}

void limpa_tela() {
    system("cls");
}

int getch_portable() {
    return _getch();
}

void sleep_ms(int ms) {
    Sleep(ms);
}

#else // LINUX
void gotoxy(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void textcolor(const char* cor) {
    printf("%s", cor);
}

void limpa_tela() {
    printf("\033[2J\033[H");
}

int getch_portable() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void sleep_ms(int ms) {
    usleep(ms * 1000);
}
#endif

void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void centraliza_texto(char *texto, int y) {
    int x = 40 - (strlen(texto) / 2);
    gotoxy(x, y);
    printf("%s", texto);
}

// === RESTANTE DO CÓDIGO (igual nos dois sistemas) ===

void inicializa_sala() {
    for(int i = 0; i < LINHAS; i++) {
        for(int j = 0; j < COLUNAS; j++) {
            sala[i][j].fileira = 'A' + i;
            sala[i][j].coluna = j + 1;
            sala[i][j].ocupada = 0;
            sala[i][j].meia = 0;
            sala[i][j].pcd = (i == 0) ? 1 : 0;
        }
    }
    total_clientes = clientes_inteira = clientes_meia = clientes_pcd = 0;
    receita_total = 0.0;
}

void desenha_tela_principal() {
    limpa_tela();
#ifdef _WIN32
    textcolor("15");  // BRANCO
#else
    textcolor(BRANCO);
#endif
    gotoxy(20, 1); printf("╔══════════════════════════════════════════════════════════════╗");
    gotoxy(20, 2); printf("║ CINEMA ESTRELA CADENTE - Sistema de Gestão de Reservas      ║");
    gotoxy(20, 3); printf("╚══════════════════════════════════════════════════════════════╝");
    tela_cinema();

    gotoxy(22, 18); textcolor(VERDE); printf("[ ]"); textcolor(BRANCO); printf(" Livre");
    gotoxy(22, 19); textcolor(VERMELHO); printf("[X]"); textcolor(BRANCO); printf(" Ocupada");
    gotoxy(22, 20); textcolor(AMARELO); printf("[P]"); textcolor(BRANCO); printf(" PCD (Fileira A)");
    gotoxy(22, 21); textcolor(CINZA); printf("Inteira: R$ %.2f | Meia: R$ %.2f", PRECO_INTEIRA, PRECO_MEIA);
    gotoxy(20, 23); printf("╔══════════════════════════════════════════════════════════════╗");
    gotoxy(20, 24); printf("║ Clientes: %3d | Inteira: %3d | Meia: %3d | PCD: %3d | Total: R$ %.2f ║",
           total_clientes, clientes_inteira, clientes_meia, clientes_pcd, receita_total);
    gotoxy(20, 25); printf("╚══════════════════════════════════════════════════════════════╝");
}

void tela_cinema() {
    gotoxy(35, 6); textcolor(CIANO);
    printf("╔══════════════════════════════════╗");
    gotoxy(35, 7); printf("║              T E L A             ║");
    gotoxy(35, 8); printf("╚══════════════════════════════════╝");
    textcolor(BRANCO);
    desenha_mapa_poltronas(-1, -1);
}

void desenha_mapa_poltronas(int hl, int hc) {
    int ix = 30, iy = 10;
    gotoxy(ix + 3, iy - 1);
    for(int c = 1; c <= COLUNAS; c++) printf(" %2d", c);

    for(int i = 0; i < LINHAS; i++) {
        gotoxy(ix, iy + i); printf("%c ", 'A' + i);
        for(int j = 0; j < COLUNAS; j++) {
            gotoxy(ix + 3 + j*3, iy + i);
            if (i == hl && j == hc) {
                textcolor(CIANO); printf("[ ]");
            } else if (sala[i][j].ocupada) {
                textcolor(VERMELHO); printf("[X]");
            } else if (sala[i][j].pcd) {
                textcolor(AMARELO); printf("[P]");
            } else {
                textcolor(VERDE); printf("[ ]");
            }
        }
    }
    textcolor(BRANCO);
}

char menu_principal() {
    char op;
    gotoxy(20, 20);
    printf("1-Reservar  2-Cancelar  3-Rel.Simples  4-Rel.Detalhado  5-Gráfico  6-Sair");
    gotoxy(40, 22); printf("Escolha: ");
    fflush(stdout);
    op = getch_portable();
    printf("%c", op);
    return op;
}

void reservar_poltrona() {
    limpa_tela();
    char fileira; int coluna, tipo; float preco;
    centraliza_texto("RESERVAR POLTRONA", 2);
    desenha_mapa_poltronas(-1, -1);

    gotoxy(25, 18); printf("Fileira (A-H): "); fflush(stdout);
    fileira = toupper(getch_portable()); printf("%c", fileira);

    gotoxy(25, 19); printf("Coluna (1-12): "); fflush(stdout);
    if (scanf("%d", &coluna) != 1) { limpar_buffer(); gotoxy(25,21); printf("Inválido!"); getch_portable(); return; }
    limpar_buffer();

    if (!valida_poltrona(fileira, coluna)) { gotoxy(25,21); printf("Poltrona inválida!"); getch_portable(); return; }

    int l = fileira - 'A', c = coluna - 1;
    if (sala[l][c].ocupada) { gotoxy(25,21); printf("Poltrona já ocupada!"); getch_portable(); return; }

    gotoxy(25,21); printf("Tipo: 1-Inteira (R$%.2f) 2-Meia (R$%.2f): ", PRECO_INTEIRA, PRECO_MEIA);
    fflush(stdout);
    tipo = getch_portable() - '0'; printf("%d", tipo);

    if (tipo != 1 && tipo != 2) { gotoxy(25,23); printf("Tipo inválido!"); getch_portable(); return; }
    if (sala[l][c].pcd && tipo == 1) { gotoxy(25,23); printf("PCD: apenas meia!"); getch_portable(); return; }

    preco = tipo == 1 ? PRECO_INTEIRA : PRECO_MEIA;
    sala[l][c].ocupada = 1; sala[l][c].meia = (tipo == 2);

    total_clientes++;
    (tipo == 1 ? clientes_inteira++ : clientes_meia++);
    if (sala[l][c].pcd) clientes_pcd++;
    receita_total += preco;

    gotoxy(25,25); printf("Reserva confirmada! %c%02d - %s - R$ %.2f", fileira, coluna, tipo==1?"Inteira":"Meia", preco);
    gotoxy(25,26); printf("Pressione qualquer tecla..."); getch_portable();
}

void cancelar_reserva() {
    limpa_tela();
    char fileira; int coluna;
    centraliza_texto("CANCELAR RESERVA", 2);
    desenha_mapa_poltronas(-1, -1);

    gotoxy(25,18); printf("Fileira (A-H): "); fflush(stdout);
    fileira = toupper(getch_portable()); printf("%c", fileira);

    gotoxy(25,19); printf("Coluna (1-12): "); fflush(stdout);
    if (scanf("%d", &coluna) != 1) { limpar_buffer(); gotoxy(25,21); printf("Inválido!"); getch_portable(); return; }
    limpar_buffer();

    if (!valida_poltrona(fileira, coluna)) { gotoxy(25,21); printf("Poltrona inválida!"); getch_portable(); return; }

    int l = fileira - 'A', c = coluna - 1;
    if (!sala[l][c].ocupada) { gotoxy(25,21); printf("Poltrona livre!"); getch_portable(); return; }

    float valor = sala[l][c].meia ? PRECO_MEIA : PRECO_INTEIRA;
    int era_meia = sala[l][c].meia, era_pcd = sala[l][c].pcd;

    sala[l][c].ocupada = sala[l][c].meia = 0;
    total_clientes--;
    (era_meia ? clientes_meia-- : clientes_inteira--);
    if (era_pcd) clientes_pcd--;
    receita_total -= valor;

    gotoxy(25,21); printf("Cancelado! Devolvido: R$ %.2f", valor);
    gotoxy(25,22); printf("Pressione qualquer tecla..."); getch_portable();
}

int valida_poltrona(char f, int c) {
    return (f >= 'A' && f <= 'H') && (c >= 1 && c <= COLUNAS);
}

void relatorio_simples() {
    limpa_tela(); centraliza_texto("RELATÓRIO SIMPLES", 2);
    gotoxy(25,5); printf("Total Clientes........: %d", total_clientes);
    gotoxy(25,6); printf("Inteira...............: %d (R$ %.2f)", clientes_inteira, clientes_inteira * PRECO_INTEIRA);
    gotoxy(25,7); printf("Meia..................: %d (R$ %.2f)", clientes_meia, clientes_meia * PRECO_MEIA);
    gotoxy(25,8); printf("PCD...................: %d", clientes_pcd);
    gotoxy(25,9); printf("Receita Total.........: R$ %.2f", receita_total);
    gotoxy(25,10); printf("Livres................: %d", LINHAS*COLUNAS - total_clientes);
    gotoxy(25,11); printf("Ocupação..............: %.1f%%", (float)total_clientes/(LINHAS*COLUNAS)*100);
    gotoxy(25,15); printf("Pressione qualquer tecla..."); getch_portable();
}

void relatorio_detalhado() {
    limpa_tela(); centraliza_texto("RELATÓRIO DETALHADO", 2);
    int y = 5, count = 0;
    for(int i=0; i<LINHAS; i++) for(int j=0; j<COLUNAS; j++) if(sala[i][j].ocupada) {
        gotoxy(25, y++); printf("%c%02d - %s%s", sala[i][j].fileira, sala[i][j].coluna,
            sala[i][j].meia?"Meia":"Inteira", sala[i][j].pcd?" (PCD)":"");
        count++;
        if(y > 22) { gotoxy(25,24); printf("Mais..."); getch_portable(); limpa_tela(); centraliza_texto("Continuação",2); y=5; }
    }
    if(!count) gotoxy(25,5); printf("Nenhuma poltrona ocupada.");
    else gotoxy(25, y+1); printf("Total: %d", count);
    gotoxy(25,23); printf("Pressione tecla..."); getch_portable();
}

void grafico_barras() {
    limpa_tela(); centraliza_texto("GRÁFICO DE OCUPAÇÃO", 2);
    int total = LINHAS * COLUNAS, max_bar = 50;
    int ocup = total_clientes, livre = total - ocup;
    float p_ocup = total ? (float)ocup/total*100 : 0;
    int b_ocup = p_ocup * max_bar / 100, b_livre = (100-p_ocup) * max_bar / 100;

    gotoxy(20,6); printf("Ocupação: %.1f%% (%d/%d)", p_ocup, ocup, total);
    gotoxy(20,8); textcolor(VERMELHO); for(int i=0;i<b_ocup;i++) printf("█"); textcolor(BRANCO); printf(" %d", ocup);
    gotoxy(20,10); textcolor(VERDE); for(int i=0;i<b_livre;i++) printf("█"); textcolor(BRANCO); printf(" %d livres", livre);

    gotoxy(20,13); printf("Inteira"); textcolor(VERMELHO);
    int bi = clientes_inteira * max_bar / total; for(int i=0;i<bi;i++) printf("█"); textcolor(BRANCO); printf(" %d", clientes_inteira);

    gotoxy(20,15); printf("Meia    "); textcolor(VERDE);
    int bm = clientes_meia * max_bar / total; for(int i=0;i<bm;i++) printf("█"); textcolor(BRANCO); printf(" %d", clientes_meia);

    gotoxy(20,17); printf("PCD     "); textcolor(AMARELO);
    int bp = clientes_pcd * max_bar / total; for(int i=0;i<bp;i++) printf("█"); textcolor(BRANCO); printf(" %d", clientes_pcd);

    gotoxy(20,20); printf("Pressione qualquer tecla..."); getch_portable();
}
