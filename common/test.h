#ifndef _TEST
#define _TEST

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>

#include "board.h"
#include "common.h"

#define SEED 501

Board * get_rand_board(int N, int M)
{
    srand(SEED);
    
    Board * board = get_board(N, M);
    
    for (int x = 0; x < N; ++x)
    {
        for (int y = 0; y < M; ++y)
            board->board[x][y] = rand() % 2;
    }
    
    return board;
}

#endif
