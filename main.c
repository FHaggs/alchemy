#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <time.h>
#include <math.h>

#ifdef WIN32
#include <windows.h> // inclui apenas no Windows
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"

// Um pixel RGBpixel (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGBpixel;

// Uma imagem RGBpixel
typedef struct
{
    int width, height;
    RGBpixel *pixels;
} Img;

// Protótipos
void load(char *name, Img *pic);
void valida();
int cmp(const void *elem1, const void *elem2);

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Funcoes do algoritimo
unsigned int should_change(RGBpixel compare_to, RGBpixel competitor_one, RGBpixel competitor_two, RGBpixel reference_2);
void change_pixels();
void improve(Img* desej, Img* output, int pos);
void switch_pixels(Img* img, int pixel1, int pixel2);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Enums para facilitar o acesso às imagens
#define ORIGEM 0
#define DESEJ 1
#define SAIDA 2

void swap(RGBpixel *a, RGBpixel *b)
{
    RGBpixel temp = *a;
    *a = *b;
    *b = temp;
}

// Function to shuffle an array of RGB pixels
void shufflePixels(RGBpixel *pixels, int size)
{
    for (int i = size - 1; i > 0; i--)
    {
        // Generate a random index between 0 and i
        int j = rand() % (i + 1);
        // Swap pixels[i] with pixels[j]
        swap(&pixels[i], &pixels[j]);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("alchemy [origem] [destino]\n");
        printf("Origem é a fonte das cores, destino é a imagem desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem com as cores
    // pic[1] -> imagem desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[ORIGEM]);
    load(argv[2], &pic[DESEJ]);

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[ORIGEM].width > pic[DESEJ].width ? pic[ORIGEM].width : pic[DESEJ].width;
    height = pic[ORIGEM].height > pic[DESEJ].height ? pic[ORIGEM].height : pic[DESEJ].height;

    // A largura e altura da imagem de saída são iguais às da imagem desejada (1)
    pic[SAIDA].width = pic[DESEJ].width;
    pic[SAIDA].height = pic[DESEJ].height;
    pic[SAIDA].pixels = malloc(pic[DESEJ].width * pic[DESEJ].height * 3); // W x H x 3 bytes (RGBpixel)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Quebra-Cabeca digital");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[ORIGEM] = SOIL_create_OGL_texture((unsigned char *)pic[ORIGEM].pixels, pic[ORIGEM].width, pic[ORIGEM].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[DESEJ] = SOIL_create_OGL_texture((unsigned char *)pic[DESEJ].pixels, pic[DESEJ].width, pic[DESEJ].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem   : %s %d x %d\n", argv[1], pic[ORIGEM].width, pic[ORIGEM].height);
    printf("Desejada : %s %d x %d\n", argv[2], pic[DESEJ].width, pic[DESEJ].height);
    sel = ORIGEM; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    srand(time(0)); // Inicializa gerador aleatório (se for usar random)

    printf("Processando...\n");

    // Copia imagem de origem na imagem de saída
    // (NUNCA ALTERAR AS IMAGENS DE ORIGEM E DESEJADA)
    int tam = pic[ORIGEM].width * pic[ORIGEM].height;
    memcpy(pic[SAIDA].pixels, pic[ORIGEM].pixels, sizeof(RGBpixel) * tam);

    //shufflePixels(pic[SAIDA].pixels, tam);
    //
    // Neste ponto, voce deve implementar o algoritmo!
    // (ou chamar funcoes para fazer isso)
    //
    // Aplica o algoritmo e gera a saida em pic[SAIDA].pixels...

    for (int i = 0; i < tam; i++)
    {
        improve(&pic[DESEJ], &pic[SAIDA], i);
    }


    // NÃO ALTERAR A PARTIR DAQUI!

    // Cria textura para a imagem de saída
    tex[SAIDA] = SOIL_create_OGL_texture((unsigned char *)pic[SAIDA].pixels, pic[SAIDA].width, pic[SAIDA].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    // Grava imagem de saída em out.bmp, para conferência
    SOIL_save_image("out.bmp", SOIL_SAVE_TYPE_BMP, pic[SAIDA].width, pic[SAIDA].height, 3, (const unsigned char *)pic[SAIDA].pixels);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

void improve(Img* desej, Img* output, int orig_rand_pixel){
    //int orig_rand_pixel = rand() % (desej->height * desej->width);
    int iter = 0;
    while(iter < 2000){
        int second_rand_pixel = rand() % (desej->height * desej->width);
        if(should_change(
            desej->pixels[orig_rand_pixel], 
            output->pixels[orig_rand_pixel], 
            output->pixels[second_rand_pixel],
            desej->pixels[second_rand_pixel]))
        {
            switch_pixels(output, orig_rand_pixel, second_rand_pixel);
        }

        
        iter++;
    }
}
void switch_pixels(Img* image, int pixel1, int pixel2){
    RGBpixel orig = image->pixels[pixel1];
    image->pixels[pixel1] = image->pixels[pixel2];
    image->pixels[pixel2] = orig;
}
// Returns 1 if should change 0 if not
unsigned int should_change(RGBpixel compare_to, RGBpixel competitor_one, RGBpixel competitor_two, RGBpixel reference_2){
    int diference1_r = competitor_one.r - compare_to.r;
    int diference1_g = competitor_one.g - compare_to.g;
    int diference1_b = competitor_one.b - compare_to.b;

    int diference2_r = competitor_two.r - compare_to.r;
    int diference2_g = competitor_two.g - compare_to.g;
    int diference2_b = competitor_two.b - compare_to.b;

    double distance1 = sqrt(pow(diference1_r, 2) + pow(diference1_g, 2) + pow(diference1_b, 2));
    double distance2 = sqrt(pow(diference2_r, 2) + pow(diference2_g, 2) + pow(diference2_b, 2));

    // Check second reference
    int diferenceref1_r = competitor_one.r - reference_2.r;
    int diferenceref1_g = competitor_one.g - reference_2.g;
    int diferenceref1_b = competitor_one.b - reference_2.b;

    int diferenceref2_r = competitor_two.r - reference_2.r;
    int diferenceref2_g = competitor_two.g - reference_2.g;
    int diferenceref2_b = competitor_two.b - reference_2.b;

    double distance3 = sqrt(pow(diferenceref1_r, 2) + pow(diferenceref1_g, 2) + pow(diferenceref1_b, 2));
    double distance4 = sqrt(pow(diferenceref2_r, 2) + pow(diferenceref2_g, 2) + pow(diferenceref2_b, 2));


    if (distance1 > distance2 && distance3 < distance4){
        return 1;
    }
    else{
        return 0;
    }

}


// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->pixels = (RGBpixel *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->pixels)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

// Verifica se o algoritmo foi aplicado corretamente:
// Ordena os pixels da imagem origem e de saída por R, G e B;
// depois compara uma com a outra: devem ser iguais
void valida()
{
    int ok = 1;
    int size = pic[ORIGEM].width * pic[ORIGEM].height;
    // Aloca memória para os dois arrays
    RGBpixel *aux1 = malloc(size * 3);
    RGBpixel *aux2 = malloc(size * 3);
    // Copia os pixels originais
    memcpy(aux1, pic[ORIGEM].pixels, size * 3);
    memcpy(aux2, pic[SAIDA].pixels, size * 3);
    // Mostra primeiros 8 pixels de ambas as imagens
    // antes de ordenar (teste)
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    printf("Validando...\n");
    // Ordena ambos os arrays
    qsort(aux1, size, sizeof(RGBpixel), cmp);
    qsort(aux2, size, sizeof(RGBpixel), cmp);
    // Mostra primeiros 8 pixels de ambas as imagens
    // depois de ordenar
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%d %d %d] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    for (int i = 0; i < size; i++)
    {
        if (aux1[i].r != aux2[i].r ||
            aux1[i].g != aux2[i].g ||
            aux1[i].b != aux2[i].b)
        {
            // Se pelo menos um dos pixels for diferente, o algoritmo foi aplicado incorretamente
            printf("*** INVÁLIDO na posição %d ***: %02X %02X %02X -> %02X %02X %02X\n",
                   i, aux1[i].r, aux1[i].g, aux1[i].b, aux2[i].r, aux2[i].g, aux2[i].b);
            ok = 0;
            break;
        }
    }
    // Libera memória dos arrays ordenados
    free(aux1);
    free(aux2);
    if (ok)
        printf(">>>> TRANSFORMAÇÃO VÁLIDA <<<<<\n");
}

// Funcao de comparacao para qsort: ordena por R, G, B (desempate nessa ordem)
int cmp(const void *elem1, const void *elem2)
{
    RGBpixel *ptr1 = (RGBpixel *)elem1;
    RGBpixel *ptr2 = (RGBpixel *)elem2;
    unsigned char r1 = ptr1->r;
    unsigned char r2 = ptr2->r;
    unsigned char g1 = ptr1->g;
    unsigned char g2 = ptr2->g;
    unsigned char b1 = ptr1->b;
    unsigned char b2 = ptr2->b;
    int r = 0;
    if (r1 < r2)
        r = -1;
    else if (r1 > r2)
        r = 1;
    else if (g1 < g2)
        r = -1;
    else if (g1 > g2)
        r = 1;
    else if (b1 < b2)
        r = -1;
    else if (b1 > b2)
        r = 1;
    return r;
}

//
// Funções de callback da OpenGL
//
// SÓ ALTERE SE VOCÊ TIVER ABSOLUTA CERTEZA DO QUE ESTÁ FAZENDO!
//

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        free(pic[0].pixels);
        free(pic[1].pixels);
        free(pic[2].pixels);
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, desejada e saída)
        sel = key - '1';
    // V para validar a solução
    if (key == 'v')
        valida();
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
