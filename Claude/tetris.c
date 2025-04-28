#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#define CLEAR_SCREEN "clear"

// Implementación de getch para sistemas UNIX
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

// Implementación de kbhit para sistemas UNIX
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
#endif

// Constantes del juego
#define TABLERO_ANCHO 12
#define TABLERO_ALTO 20
#define MAX_PUNTAJES 10
#define ARCHIVO_PUNTAJES "puntajes.json"

// Definición de colores (para sistemas Windows)
#define COLOR_RESET "\033[0m"
#define COLOR_CYAN "\033[36m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_WHITE "\033[37m"

// Estructura para las piezas de Tetris
typedef struct
{
    int forma[4][4];
    int ancho;
    int color;
} Pieza;

// Estructura para el jugador y su puntuación
typedef struct
{
    char nombre[50];
    int puntuacion;
    char fecha[20];
} Puntaje;

// Variables globales
int tablero[TABLERO_ALTO][TABLERO_ANCHO] = {0};
Pieza piezaActual;
int piezaX, piezaY;
int nivel = 1;
int lineasEliminadas = 0;
int puntuacion = 0;
Puntaje puntajes[MAX_PUNTAJES];
int numPuntajes = 0;

// Definición de las piezas de Tetris (los 7 tetriminos)
Pieza tetriminos[7] = {
    // I - Pieza cyan
    {{{0, 0, 0, 0},
      {1, 1, 1, 1},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     4,
     1},
    // J - Pieza azul
    {{{1, 0, 0, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     3,
     2},
    // L - Pieza naranja
    {{{0, 0, 1, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     3,
     3},
    // O - Pieza amarilla
    {{{1, 1, 0, 0},
      {1, 1, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     2,
     4},
    // S - Pieza verde
    {{{0, 1, 1, 0},
      {1, 1, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     3,
     5},
    // T - Pieza magenta
    {{{0, 1, 0, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     3,
     6},
    // Z - Pieza roja
    {{{1, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}},
     3,
     7}};

// Prototipos de funciones
void inicializarTablero();
void generarNuevaPieza();
void dibujarTablero();
void dibujarPieza();
bool moverPiezaAbajo();
void moverPiezaIzquierda();
void moverPiezaDerecha();
void rotarPieza();
bool verificarColision(int x, int y, int forma[4][4]);
void fijarPieza();
int eliminarLineasCompletas();
void actualizarPuntuacion(int lineas);
void guardarPuntajes();
void cargarPuntajes();
void mostrarPuntajes();
void agregarPuntaje(int puntos);
void pausa(int milisegundos);
char *obtenerColorAnsi(int colorId);
void juegoTerminado();

// Implementación de funciones principales
void inicializarTablero()
{
    for (int i = 0; i < TABLERO_ALTO; i++)
    {
        for (int j = 0; j < TABLERO_ANCHO; j++)
        {
            if (j == 0 || j == TABLERO_ANCHO - 1 || i == TABLERO_ALTO - 1)
            {
                tablero[i][j] = -1; // Bordes del tablero
            }
            else
            {
                tablero[i][j] = 0; // Espacio vacío
            }
        }
    }
}

void generarNuevaPieza()
{
    // Seleccionar una pieza aleatoria
    piezaActual = tetriminos[rand() % 7];

    // Posición inicial (centrada en la parte superior del tablero)
    piezaX = TABLERO_ANCHO / 2 - piezaActual.ancho / 2;
    piezaY = 0;

    // Verificar si la pieza puede ser colocada (fin del juego)
    if (verificarColision(piezaX, piezaY, piezaActual.forma))
    {
        juegoTerminado();
    }
}

void dibujarTablero()
{
    system(CLEAR_SCREEN);

    printf("\n  === TETRIS EN CONSOLA ===\n\n");
    printf("  Puntuación: %d   Nivel: %d   Líneas: %d\n\n", puntuacion, nivel, lineasEliminadas);

    // Copiar el tablero en un buffer temporal para dibujar la pieza actual
    int tempTablero[TABLERO_ALTO][TABLERO_ANCHO];
    memcpy(tempTablero, tablero, sizeof(tablero));

    // Dibujar la pieza actual en el buffer
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (piezaActual.forma[i][j] &&
                piezaY + i >= 0 && piezaY + i < TABLERO_ALTO &&
                piezaX + j >= 0 && piezaX + j < TABLERO_ANCHO)
            {
                tempTablero[piezaY + i][piezaX + j] = piezaActual.color;
            }
        }
    }

    // Dibujar el tablero con la pieza
    printf("  ");
    for (int j = 0; j < TABLERO_ANCHO; j++)
    {
        printf("--");
    }
    printf("\n");

    for (int i = 0; i < TABLERO_ALTO - 1; i++)
    { // No dibujamos la última fila (base)
        printf("  |");
        for (int j = 1; j < TABLERO_ANCHO - 1; j++)
        {
            if (tempTablero[i][j] == 0)
            {
                printf("  ");
            }
            else if (tempTablero[i][j] == -1)
            {
                printf("| ");
            }
            else
            {
                // Usar colores si están disponibles
                printf("%s■ %s", obtenerColorAnsi(tempTablero[i][j]), COLOR_RESET);
            }
        }
        printf("|\n");
    }

    printf("  ");
    for (int j = 0; j < TABLERO_ANCHO; j++)
    {
        printf("--");
    }
    printf("\n");

    printf("\n  Controles: A/◄ - Izquierda, D/► - Derecha, W/▲ - Rotar, S/▼ - Bajar, Q - Salir\n");
}

char *obtenerColorAnsi(int colorId)
{
    switch (colorId)
    {
    case 1:
        return COLOR_CYAN;
    case 2:
        return COLOR_BLUE;
    case 3:
        return COLOR_YELLOW;
    case 4:
        return COLOR_YELLOW;
    case 5:
        return COLOR_GREEN;
    case 6:
        return COLOR_MAGENTA;
    case 7:
        return COLOR_RED;
    default:
        return COLOR_WHITE;
    }
}

bool verificarColision(int x, int y, int forma[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (forma[i][j])
            {
                int posX = x + j;
                int posY = y + i;

                // Verificar límites y colisiones
                if (posX < 0 || posX >= TABLERO_ANCHO || posY >= TABLERO_ALTO ||
                    (posY >= 0 && tablero[posY][posX] != 0))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

bool moverPiezaAbajo()
{
    if (!verificarColision(piezaX, piezaY + 1, piezaActual.forma))
    {
        piezaY++;
        return true;
    }
    else
    {
        fijarPieza();
        int lineas = eliminarLineasCompletas();
        actualizarPuntuacion(lineas);
        generarNuevaPieza();
        return false;
    }
}

void moverPiezaIzquierda()
{
    if (!verificarColision(piezaX - 1, piezaY, piezaActual.forma))
    {
        piezaX--;
    }
}

void moverPiezaDerecha()
{
    if (!verificarColision(piezaX + 1, piezaY, piezaActual.forma))
    {
        piezaX++;
    }
}

void rotarPieza()
{
    // Crear una copia temporal de la pieza para rotar
    int tempForma[4][4] = {0};

    // Rotar 90 grados en sentido horario
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            tempForma[j][3 - i] = piezaActual.forma[i][j];
        }
    }

    // Verificar si la rotación provoca colisión
    if (!verificarColision(piezaX, piezaY, tempForma))
    {
        // Aplicar la rotación
        memcpy(piezaActual.forma, tempForma, sizeof(tempForma));
    }
}

void fijarPieza()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (piezaActual.forma[i][j])
            {
                int posY = piezaY + i;
                int posX = piezaX + j;

                if (posY >= 0 && posX >= 0 && posY < TABLERO_ALTO && posX < TABLERO_ANCHO)
                {
                    tablero[posY][posX] = piezaActual.color;
                }
            }
        }
    }
}

int eliminarLineasCompletas()
{
    int lineasEliminadas = 0;

    for (int i = 0; i < TABLERO_ALTO - 1; i++)
    {
        bool lineaCompleta = true;

        // Verificar si la línea está completa
        for (int j = 1; j < TABLERO_ANCHO - 1; j++)
        {
            if (tablero[i][j] == 0)
            {
                lineaCompleta = false;
                break;
            }
        }

        // Si la línea está completa, eliminarla
        if (lineaCompleta)
        {
            lineasEliminadas++;

            // Mover todas las líneas superiores hacia abajo
            for (int k = i; k > 0; k--)
            {
                for (int j = 1; j < TABLERO_ANCHO - 1; j++)
                {
                    tablero[k][j] = tablero[k - 1][j];
                }
            }

            // Limpiar la línea superior
            for (int j = 1; j < TABLERO_ANCHO - 1; j++)
            {
                tablero[0][j] = 0;
            }

            // Como la línea actual se ha movido hacia abajo, verificarla de nuevo
            i--;
        }
    }

    return lineasEliminadas;
}

void actualizarPuntuacion(int lineas)
{
    if (lineas > 0)
    {
        // Sistema de puntuación básico:
        // 1 línea = 100 puntos, 2 líneas = 300 puntos, 3 líneas = 500 puntos, 4 líneas (Tetris) = 800 puntos
        int puntosPorLinea;
        switch (lineas)
        {
        case 1:
            puntosPorLinea = 100;
            break;
        case 2:
            puntosPorLinea = 300;
            break;
        case 3:
            puntosPorLinea = 500;
            break;
        case 4:
            puntosPorLinea = 800;
            break;
        default:
            puntosPorLinea = 0;
        }

        puntuacion += puntosPorLinea * nivel;
        lineasEliminadas += lineas;

        // Actualizar nivel (cada 10 líneas)
        nivel = 1 + (lineasEliminadas / 10);
    }
}

void juegoTerminado()
{
    system(CLEAR_SCREEN);
    printf("\n\n  === JUEGO TERMINADO ===\n\n");
    printf("  Tu puntuación final: %d\n", puntuacion);
    printf("  Nivel alcanzado: %d\n", nivel);
    printf("  Líneas eliminadas: %d\n\n", lineasEliminadas);

    agregarPuntaje(puntuacion);
    guardarPuntajes();
    mostrarPuntajes();

    printf("\n  Presiona cualquier tecla para salir...");
    getch();
    exit(0);
}

void pausa(int milisegundos)
{
#ifdef _WIN32
    Sleep(milisegundos);
#else
    usleep(milisegundos * 1000);
#endif
}

// Implementación para manejo de puntajes
void agregarPuntaje(int puntos)
{
    if (puntos <= 0)
        return;

    char nombre[50];
    printf("\n  Ingresa tu nombre (max 20 caracteres): ");
    scanf("%49s", nombre);

    // Crear nuevo puntaje
    Puntaje nuevoPuntaje;
    strncpy(nuevoPuntaje.nombre, nombre, sizeof(nuevoPuntaje.nombre) - 1);
    nuevoPuntaje.nombre[sizeof(nuevoPuntaje.nombre) - 1] = '\0';
    nuevoPuntaje.puntuacion = puntos;

    // Obtener fecha actual
    time_t ahora = time(NULL);
    struct tm *tm_info = localtime(&ahora);
    strftime(nuevoPuntaje.fecha, sizeof(nuevoPuntaje.fecha), "%d/%m/%Y", tm_info);

    // Insertar el nuevo puntaje en orden descendente
    int i;
    for (i = 0; i < numPuntajes; i++)
    {
        if (puntos > puntajes[i].puntuacion)
        {
            // Desplazar los puntajes más bajos
            for (int j = numPuntajes; j > i; j--)
            {
                if (j < MAX_PUNTAJES)
                {
                    puntajes[j] = puntajes[j - 1];
                }
            }
            break;
        }
    }

    // Añadir el nuevo puntaje si hay espacio
    if (i < MAX_PUNTAJES)
    {
        puntajes[i] = nuevoPuntaje;
        if (numPuntajes < MAX_PUNTAJES)
            numPuntajes++;
    }
}

void guardarPuntajes()
{
    FILE *archivo = fopen(ARCHIVO_PUNTAJES, "w");
    if (archivo == NULL)
    {
        printf("Error: No se pudo abrir el archivo de puntajes para escribir.\n");
        return;
    }

    // Escribir inicio del JSON
    fprintf(archivo, "{\n  \"puntajes\": [\n");

    // Escribir cada puntaje
    for (int i = 0; i < numPuntajes; i++)
    {
        fprintf(archivo, "    {\n");
        fprintf(archivo, "      \"nombre\": \"%s\",\n", puntajes[i].nombre);
        fprintf(archivo, "      \"puntuacion\": %d,\n", puntajes[i].puntuacion);
        fprintf(archivo, "      \"fecha\": \"%s\"\n", puntajes[i].fecha);
        fprintf(archivo, "    }%s\n", (i < numPuntajes - 1) ? "," : "");
    }

    // Cerrar el JSON
    fprintf(archivo, "  ]\n}");

    fclose(archivo);
}

void cargarPuntajes()
{
    FILE *archivo = fopen(ARCHIVO_PUNTAJES, "r");
    if (archivo == NULL)
    {
        // El archivo no existe o no se puede abrir, inicializar con una lista vacía
        numPuntajes = 0;
        return;
    }

    // Para un parser JSON real, deberías usar una biblioteca como cJSON
    // Esta es una implementación simple para este ejemplo básico

    char linea[256];
    numPuntajes = 0;

    // Buscar el inicio de la lista de puntajes
    while (fgets(linea, sizeof(linea), archivo) && numPuntajes < MAX_PUNTAJES)
    {
        if (strstr(linea, "\"nombre\"") != NULL)
        {
            // Extraer nombre
            char *inicio = strchr(linea, ':') + 3; // Saltamos ': "'
            char *fin = strrchr(linea, '"');
            if (inicio && fin && fin > inicio)
            {
                size_t longitud = fin - inicio;
                strncpy(puntajes[numPuntajes].nombre, inicio, longitud);
                puntajes[numPuntajes].nombre[longitud] = '\0';
            }

            // Extraer puntuación
            if (fgets(linea, sizeof(linea), archivo))
            {
                char *puntoPos = strchr(linea, ':');
                if (puntoPos)
                {
                    puntajes[numPuntajes].puntuacion = atoi(puntoPos + 1);
                }
            }

            // Extraer fecha
            if (fgets(linea, sizeof(linea), archivo))
            {
                char *inicio = strchr(linea, ':') + 3; // Saltamos ': "'
                char *fin = strrchr(linea, '"');
                if (inicio && fin && fin > inicio)
                {
                    size_t longitud = fin - inicio;
                    strncpy(puntajes[numPuntajes].fecha, inicio, longitud);
                    puntajes[numPuntajes].fecha[longitud] = '\0';
                }
            }

            numPuntajes++;
        }
    }

    fclose(archivo);
}

void mostrarPuntajes()
{
    printf("\n  === MEJORES PUNTAJES ===\n\n");
    printf("  %-20s %-10s %-10s\n", "Nombre", "Puntos", "Fecha");
    printf("  ---------------------------------------\n");

    for (int i = 0; i < numPuntajes; i++)
    {
        printf("  %-20s %-10d %-10s\n",
               puntajes[i].nombre,
               puntajes[i].puntuacion,
               puntajes[i].fecha);
    }
}

// Función principal
int main()
{
    // Inicializar la semilla para números aleatorios
    srand((unsigned int)time(NULL));

    // Inicializar el tablero
    inicializarTablero();

    // Cargar puntajes previos
    cargarPuntajes();

    // Generar la primera pieza
    generarNuevaPieza();

    // Variables para el bucle principal
    bool salir = false;
    int tecla;
    int tiempoEspera = 500; // Tiempo inicial entre caídas (ms)
    clock_t tiempoUltimaCaida = clock();

    // Bucle principal del juego
    while (!salir)
    {
        // Dibujar el estado actual
        dibujarTablero();

        // Comprobar si hay entrada del teclado
        if (kbhit())
        {
            tecla = tolower(getch());

            switch (tecla)
            {
            case 'a':
            case 75: // Flecha izquierda
                moverPiezaIzquierda();
                break;
            case 'd':
            case 77: // Flecha derecha
                moverPiezaDerecha();
                break;
            case 'w':
            case 72: // Flecha arriba
                rotarPieza();
                break;
            case 's':
            case 80: // Flecha abajo
                moverPiezaAbajo();
                break;
            case 'q':
                salir = true;
                break;
            }
        }

        // Actualizar la caída de la pieza basada en el tiempo
        clock_t tiempoActual = clock();
        if ((tiempoActual - tiempoUltimaCaida) > (tiempoEspera / nivel))
        {
            moverPiezaAbajo();
            tiempoUltimaCaida = tiempoActual;
        }

        // Pequeña pausa para reducir consumo de CPU
        pausa(30);
    }

    return 0;
}