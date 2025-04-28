#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h> // Para _kbhit() y _getch() en Windows. Usa termios si estás en Linux.

#define WIDTH 10
#define HEIGHT 20

char board[HEIGHT][WIDTH];

// Tetrominós simples (representados como 4 posiciones x,y relativas)
typedef struct
{
    int x, y;
} Block;

typedef struct
{
    Block blocks[4];
} Tetromino;

Tetromino tetrominos[] = {
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}},   // O
    {{{0, 0}, {-1, 0}, {1, 0}, {2, 0}}},  // I
    {{{0, 0}, {-1, 0}, {0, 1}, {1, 1}}},  // S
    {{{0, 0}, {1, 0}, {0, 1}, {-1, 1}}},  // Z
    {{{0, 0}, {-1, 0}, {1, 0}, {1, 1}}},  // L
    {{{0, 0}, {1, 0}, {-1, 0}, {-1, 1}}}, // J
    {{{0, 0}, {-1, 0}, {1, 0}, {0, 1}}}   // T
};

Tetromino current;
int posX = 5, posY = 0;
int score = 0;

void initBoard()
{
    memset(board, ' ', sizeof(board));
}

void drawBoard()
{
    system("cls"); // En Linux sería "clear"
    for (int y = 0; y < HEIGHT; y++)
    {
        printf("|");
        for (int x = 0; x < WIDTH; x++)
        {
            char cell = board[y][x];
            int isBlock = 0;
            for (int i = 0; i < 4; i++)
            {
                if (posX + current.blocks[i].x == x && posY + current.blocks[i].y == y)
                {
                    isBlock = 1;
                    break;
                }
            }
            if (isBlock)
                printf("#");
            else
                printf("%c", cell);
        }
        printf("|\n");
    }
    printf("Score: %d\n", score);
}

int checkCollision(int newX, int newY)
{
    for (int i = 0; i < 4; i++)
    {
        int x = newX + current.blocks[i].x;
        int y = newY + current.blocks[i].y;
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
            return 1;
        if (board[y][x] != ' ')
            return 1;
    }
    return 0;
}

void mergePiece()
{
    for (int i = 0; i < 4; i++)
    {
        int x = posX + current.blocks[i].x;
        int y = posY + current.blocks[i].y;
        if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH)
            board[y][x] = '#';
    }
}

void clearLines()
{
    for (int y = HEIGHT - 1; y >= 0; y--)
    {
        int full = 1;
        for (int x = 0; x < WIDTH; x++)
        {
            if (board[y][x] == ' ')
            {
                full = 0;
                break;
            }
        }
        if (full)
        {
            for (int yy = y; yy > 0; yy--)
            {
                memcpy(board[yy], board[yy - 1], WIDTH);
            }
            memset(board[0], ' ', WIDTH);
            score += 100;
            y++; // Volver a revisar esta línea
        }
    }
}

void saveScore()
{
    FILE *f = fopen("puntuacion.json", "w");
    if (f)
    {
        fprintf(f, "{ \"score\": %d }\n", score);
        fclose(f);
    }
}

int main()
{
    srand(time(NULL));
    initBoard();
    current = tetrominos[rand() % 7];
    int tick = 0;

    while (1)
    {
        if (_kbhit())
        {
            char c = _getch();
            if (c == 'a' && !checkCollision(posX - 1, posY))
                posX--;
            if (c == 'd' && !checkCollision(posX + 1, posY))
                posX++;
            if (c == 's' && !checkCollision(posX, posY + 1))
                posY++;
        }

        if (tick % 10 == 0)
        { // Cada 10 ticks baja una fila
            if (!checkCollision(posX, posY + 1))
            {
                posY++;
            }
            else
            {
                mergePiece();
                clearLines();
                posX = 5;
                posY = 0;
                current = tetrominos[rand() % 7];
                if (checkCollision(posX, posY))
                {
                    drawBoard();
                    printf("GAME OVER!\n");
                    saveScore();
                    break;
                }
            }
        }

        drawBoard();
        _sleep(50); // 50ms
        tick++;
    }

    return 0;
}