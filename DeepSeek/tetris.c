#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>   // Para getch() en Windows
#include <windows.h> // Para Sleep() y system("cls")

#define WIDTH 10
#define HEIGHT 20
#define BLOCK_SIZE 4

// Colores (solo para Windows)
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_YELLOW 6
#define COLOR_WHITE 7

// Piezas del Tetris
int pieces[7][4][4] = {
    // I
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // J
    {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // L
    {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // O
    {
        {0, 0, 0, 0},
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}},
    // S
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // T
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}},
    // Z
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}}};

int board[HEIGHT][WIDTH] = {0};
int current_piece[BLOCK_SIZE][BLOCK_SIZE] = {0};
int current_x = 0, current_y = 0;
int current_type = 0;
int score = 0;
int game_over = 0;

// Funciones
void init_game();
void draw_board();
void draw_piece(int x, int y, int piece[BLOCK_SIZE][BLOCK_SIZE]);
void new_piece();
int check_collision(int x, int y, int piece[BLOCK_SIZE][BLOCK_SIZE]);
void merge_piece();
void rotate_piece();
void clear_lines();
void update_game();
void save_score();
void load_scores();
void set_color(int color);

int main()
{
    init_game();

    while (!game_over)
    {
        if (_kbhit())
        {
            char key = _getch();
            switch (key)
            {
            case 'a':
            case 'A':
                if (!check_collision(current_x - 1, current_y, current_piece))
                    current_x--;
                break;
            case 'd':
            case 'D':
                if (!check_collision(current_x + 1, current_y, current_piece))
                    current_x++;
                break;
            case 's':
            case 'S':
                if (!check_collision(current_x, current_y + 1, current_piece))
                    current_y++;
                break;
            case 'w':
            case 'W':
                rotate_piece();
                break;
            case 'q':
            case 'Q':
                game_over = 1;
                break;
            }
        }

        update_game();
        draw_board();
        Sleep(300); // Velocidad del juego
    }

    printf("Game Over! Puntuacion: %d\n", score);
    save_score();
    printf("Puntuaciones guardadas en scores.txt\n");

    return 0;
}

void init_game()
{
    srand(time(NULL));
    new_piece();
    system("cls");
}

void draw_board()
{
    system("cls");

    printf("Tetris - Puntuacion: %d\n", score);
    printf("Controles: WASD (movimiento), W (rotar), Q (salir)\n\n");

    // Dibujar el tablero
    for (int y = 0; y < HEIGHT; y++)
    {
        printf("|");
        for (int x = 0; x < WIDTH; x++)
        {
            if (board[y][x])
            {
                set_color(board[y][x]);
                printf("[]");
                set_color(COLOR_WHITE);
            }
            else
            {
                printf("  ");
            }
        }
        printf("|\n");
    }

    // Dibujar el borde inferior
    printf("+");
    for (int x = 0; x < WIDTH; x++)
    {
        printf("--");
    }
    printf("+\n");
}

void draw_piece(int x, int y, int piece[BLOCK_SIZE][BLOCK_SIZE])
{
    for (int py = 0; py < BLOCK_SIZE; py++)
    {
        for (int px = 0; px < BLOCK_SIZE; px++)
        {
            if (piece[py][px])
            {
                int board_y = y + py;
                int board_x = x + px;
                if (board_y >= 0 && board_y < HEIGHT && board_x >= 0 && board_x < WIDTH)
                {
                    board[board_y][board_x] = piece[py][px];
                }
            }
        }
    }
}

void new_piece()
{
    current_type = rand() % 7;
    current_x = WIDTH / 2 - BLOCK_SIZE / 2;
    current_y = 0;

    // Copiar la pieza actual
    for (int y = 0; y < BLOCK_SIZE; y++)
    {
        for (int x = 0; x < BLOCK_SIZE; x++)
        {
            current_piece[y][x] = pieces[current_type][y][x] * (current_type + 1);
        }
    }

    // Comprobar si hay colisión al crear una nueva pieza (game over)
    if (check_collision(current_x, current_y, current_piece))
    {
        game_over = 1;
    }
}

int check_collision(int x, int y, int piece[BLOCK_SIZE][BLOCK_SIZE])
{
    for (int py = 0; py < BLOCK_SIZE; py++)
    {
        for (int px = 0; px < BLOCK_SIZE; px++)
        {
            if (piece[py][px])
            {
                int board_y = y + py;
                int board_x = x + px;

                // Comprobar límites
                if (board_x < 0 || board_x >= WIDTH || board_y >= HEIGHT)
                {
                    return 1;
                }

                // Comprobar colisión con otras piezas
                if (board_y >= 0 && board[board_y][board_x])
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void merge_piece()
{
    for (int y = 0; y < BLOCK_SIZE; y++)
    {
        for (int x = 0; x < BLOCK_SIZE; x++)
        {
            if (current_piece[y][x])
            {
                int board_y = current_y + y;
                int board_x = current_x + x;
                if (board_y >= 0 && board_y < HEIGHT && board_x >= 0 && board_x < WIDTH)
                {
                    board[board_y][board_x] = current_piece[y][x];
                }
            }
        }
    }
}

void rotate_piece()
{
    int temp[BLOCK_SIZE][BLOCK_SIZE] = {0};

    // Rotar la pieza
    for (int y = 0; y < BLOCK_SIZE; y++)
    {
        for (int x = 0; x < BLOCK_SIZE; x++)
        {
            temp[x][BLOCK_SIZE - 1 - y] = current_piece[y][x];
        }
    }

    // Comprobar si la rotación es válida
    if (!check_collision(current_x, current_y, temp))
    {
        for (int y = 0; y < BLOCK_SIZE; y++)
        {
            for (int x = 0; x < BLOCK_SIZE; x++)
            {
                current_piece[y][x] = temp[y][x];
            }
        }
    }
}

void clear_lines()
{
    int lines_cleared = 0;

    for (int y = HEIGHT - 1; y >= 0; y--)
    {
        int line_complete = 1;
        for (int x = 0; x < WIDTH; x++)
        {
            if (!board[y][x])
            {
                line_complete = 0;
                break;
            }
        }

        if (line_complete)
        {
            lines_cleared++;
            // Mover todas las líneas hacia abajo
            for (int ny = y; ny > 0; ny--)
            {
                for (int x = 0; x < WIDTH; x++)
                {
                    board[ny][x] = board[ny - 1][x];
                }
            }
            // Limpiar la línea superior
            for (int x = 0; x < WIDTH; x++)
            {
                board[0][x] = 0;
            }
            y++; // Revisar la misma línea otra vez
        }
    }

    // Actualizar puntuación
    switch (lines_cleared)
    {
    case 1:
        score += 100;
        break;
    case 2:
        score += 300;
        break;
    case 3:
        score += 500;
        break;
    case 4:
        score += 800;
        break;
    }
}

void update_game()
{
    // Borrar la pieza actual del tablero
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (board[y][x] > 0)
            {
                board[y][x] = 0;
            }
        }
    }

    // Mover la pieza hacia abajo
    if (!check_collision(current_x, current_y + 1, current_piece))
    {
        current_y++;
    }
    else
    {
        merge_piece();
        clear_lines();
        new_piece();
    }

    // Dibujar la pieza actual
    draw_piece(current_x, current_y, current_piece);
}

void save_score()
{
    FILE *file = fopen("scores.txt", "a");
    if (file)
    {
        time_t now;
        time(&now);
        struct tm *local = localtime(&now);
        fprintf(file, "Fecha: %02d/%02d/%d - Puntuacion: %d\n",
                local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, score);
        fclose(file);
    }
}

void load_scores()
{
    FILE *file = fopen("scores.txt", "r");
    if (file)
    {
        printf("\n--- Historial de Puntuaciones ---\n");
        char line[100];
        while (fgets(line, sizeof(line), file))
        {
            printf("%s", line);
        }
        fclose(file);
    }
}

// Solo funciona en Windows
void set_color(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}