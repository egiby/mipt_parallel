#ifndef _BOARD
#define _BOARD

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include "common.h"

typedef struct Board
{
    int N, M;
    int ** board;
} Board;

Board * get_board(int n, int m)
{
    Board * board = malloc(sizeof(Board));
    board->N = n;
    board->M = m;
    board->board = malloc(n * sizeof(int*));
    for (int i = 0; i < n; ++i)
        board->board[i] = malloc(m * sizeof(int));
    
    return board;
}

void read_board(Board * board, FILE * file)
{
    for (int x = 0; x < board->N; ++x)
    {
        for (int y = 0; y < board->M; ++y)
            board->board[x][y] = getc(file) - '0';
        getc(file);
    }
}

void print_board(Board * board)
{
    for (int x = 0; x < board->N; ++x)
    {
        for (int y = 0; y < board->M; ++y)
            printf("%d", board->board[x][y]);
        printf("\n");
    }
    printf("\n");
}

int get_elem(Board * board, int x, int y)
{
    x = (x + board->N) % board->N;
    y = (y + board->M) % board->M;
    
    return board->board[x][y];
}

void delete_board(Board *);

void reverse_board(Board * board)
{
    int ** new_board = malloc(board->M * sizeof(int*));
    for (int i = 0; i < board->M; ++i)
        new_board[i] = malloc(board->N * sizeof(int));
    
    for (int x = 0; x < board->N; ++x)
        for (int y = 0; y < board->M; ++y)
            new_board[y][x] = board->board[x][y];
    
    delete_board(board);
    
    swap_int(&board->N, &board->M);
    board->board = new_board;
}

void delete_board(Board * board)
{
    for (int i = 0; i < board->N; ++i)
        free(board->board[i]);
    free(board->board);
}

#endif
