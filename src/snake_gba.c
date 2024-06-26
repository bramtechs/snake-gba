/*
 * hello.c
 * this program is a simple GBA example
 * it simply creates a colored screen and waits
 */

#define ushort unsigned short
#define uchar unsigned char

/* the width and height of the screen */
#define WIDTH 240
#define HEIGHT 160

#define CELLS_X 24
#define CELLS_Y 16
#define CELL_SIZE 10

#define MAX_LEN 100

/* these identifiers define different bit positions of the display control */
#define MODE3 0x0003
#define BG2 0x0400

#include <stdbool.h>
#include <stddef.h>

#include "common.h"

// TODO: find a way to use Mode 5 with smaller resolution

/* the screen is simply a pointer into memory at a specific address this
 * pointer points to 16-bit colors of which there are 240x160 */
M_IWRAM_CONST_VAR volatile unsigned short* screen = (volatile unsigned short*) 0x6000000;

/* the display control pointer points to the gba graphics register */
M_IWRAM_CONST_VAR volatile unsigned long* display_control = (volatile unsigned long*) 0x4000000;

M_IWRAM_INITIALIZED_VAR unsigned long clock = 0;

/* compute a 16-bit integer color based on the three components */
unsigned short make_color(unsigned char r, unsigned char g, unsigned char b) {
    unsigned short color = (b & 0x1f) << 10;
    color |= (g & 0x1f) << 5;
    color |= (r & 0x1f);
    return color;
}

/* place a pixel of a given color on the screen */
void put_pixel(int row, int col, unsigned short color) {
    /* set the screen location to this color */
    screen[row * WIDTH + col] = color;
}

void put_cell(int x, int y, ushort col)
{
    for (int yy = y*CELL_SIZE; yy < (y+1)*CELL_SIZE; yy++) {
        for (int xx = x*CELL_SIZE; xx < (x+1)*CELL_SIZE; xx++) {
            put_pixel(yy, xx, col);
        }
    }
}

int clamp(int val, int a, int b) {
    if (val < a) {
        return a;
    }
    if (val > b) {
        return b;
    }
    return val;
}


#define COLOR_COUNT 6
M_IWRAM_CONST_VAR int COLORS[COLOR_COUNT][3] = {
    {	255,	0,	0 },
    {    255,	255,	0},
    {0,	255,	0},
    {    0,	255,	255},
    {	0,	0,	255},
    {    255,	0,	255}
};

typedef struct {
    int x;
    int y;
} piece;

typedef struct {   
    int piece_count;
    piece pieces[MAX_LEN];
} snake;

void snake_queue(snake* s, int x, int y)
{
    if (s->piece_count < MAX_LEN)
    {
        s->pieces[s->piece_count++] = (piece) {
            .x = x,
            .y = y
        };
    }
}

void snake_dequeue(snake* s)
{
    for (int i = 0; i < s->piece_count-1; i++)
    {
        s->pieces[i] = s->pieces[i+1];
    }
    s->piece_count--;
}

void snake_init(snake* s)
{
    snake_queue(s, CELLS_X / 2, CELLS_Y / 2);
    snake_queue(s, CELLS_X / 2 - 1, CELLS_Y / 2);
    snake_queue(s, CELLS_X / 2 - 2, CELLS_Y / 2);
}

void snake_move(snake* s, int dx, int dy)
{
    // snake_dequeue(s);

    // peek front
    piece head = s->pieces[s->piece_count-1];
    
    // push new head piece
    int x = (head.x + dx) % CELLS_X;
    int y = (head.y + dy) % CELLS_Y;
    if (x < 0) x = CELLS_X - 1;
    if (y < 0) y = CELLS_Y - 1;
    
    snake_queue(s, x, y);
}

void snake_draw(snake* s)
{
    for (int i = 0; i < s->piece_count; i++)
    {
        int ci = i % COLOR_COUNT;
        ushort col = make_color(COLORS[ci][0],COLORS[ci][1],COLORS[ci][2]);
        put_cell(s->pieces[i].x, s->pieces[i].y, col);
    }
}

int main() { 
    /* we set the mode to mode 3 with background 2 on */
    *display_control = MODE3 | BG2;

    snake snake = {0};
    snake_init(&snake);

    int c = 0;
    
    /* we now loop forever displaying the image */
    while (1) {

        c = clock % 20;
        if (c < 3)
            snake_move(&snake, 1, 0);
        else
            snake_move(&snake, 0, 1);

        // TODO: use memset
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                put_pixel(y, x, 0);
            }   
        }
        
        snake_draw(&snake);
        
        clock++;
    }
}

