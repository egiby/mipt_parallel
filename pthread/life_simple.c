#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include "../common/board.h"
#include "../common/common.h"
#include "../common/cyclic_barrier.h"

Board * new_board;

int NUMBER_OF_STEPS = 0;
int NUMBER_OF_THREADS = 1;

FILE * in_file;

CyclicBarrier *bottom_barrier, *top_barrier;

void success_swap(void ** argv)
{
    swap_iters2((int ***)argv[0], (int ***)argv[1]);
}

void void_f(void ** argv)
{
    return;
}

void * recalc_part(void * args)
{
    void ** argv = (void**)args;
    Board * board = (Board*)argv[0];
    int l = *(int*)argv[1];
    int r = *(int*)argv[2];
    
    for (int k = 0; k < NUMBER_OF_STEPS; ++k)
    {
        lock_barrier(top_barrier, NULL);
        for (int x = l; x < r; ++x)
            for (int y = 0; y < board->M; ++y)
            {
                new_board->board[x][y] = get_life_value_by_board(x, y, board);
            }
            
        void ** argv = malloc(sizeof(void*) * 2);
        argv[0] = &board->board;
        argv[1] = &new_board->board;
        lock_barrier(bottom_barrier, argv);
        free(argv);
    }
    
    return NULL;
}

void play(Board * board)
{
    pthread_t * t = malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);
    
    int diff = board->N / NUMBER_OF_THREADS;
    int rem = board->N % NUMBER_OF_THREADS;
    int start = 0;
    int end = 0;
    
    top_barrier = get_cyclic_barrier(NUMBER_OF_THREADS, &void_f);
    bottom_barrier = get_cyclic_barrier(NUMBER_OF_THREADS, &success_swap);
    
    for (int i = 0; i < NUMBER_OF_THREADS; ++i)
    {
        void ** argv = malloc(sizeof(void*) * 3);
        start = end;
        end += diff + ((rem--) > 0);
        int * l = malloc(sizeof(int));
        *l = start;
        int * r = malloc(sizeof(int));
        *r = end;
        
        argv[0] = (void*)board;
        argv[1] = (void*)l;
        argv[2] = (void*)r;
        pthread_create(&t[i], NULL, recalc_part, argv);
    }
    
    for (int i = 0; i < NUMBER_OF_THREADS; ++i)
    {
        pthread_join(t[i], NULL);
    }
}

int main(int argc, char ** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "It must be board's filename and number of threads\n");
        return 0;
    }
    FILE * file = fopen(argv[1], "r");
    NUMBER_OF_THREADS = atoi(argv[2]);
    
    int n, m, k;
    char is_reversed = 0;
    fscanf(file, "%d %d %d\n", &n, &m, &k);
    
    Board * board = get_board(n, m);
    read_board(board, file);
    
    if (n < m)
    {
        is_reversed = 1;
        reverse_board(board);
        swap_int(&n, &m);
    }
    
    new_board = get_board(n, m);
    NUMBER_OF_STEPS = k;
    
    play(board);
    
    if (is_reversed)
    {
        reverse_board(board);
        swap_int(&n, &m);
    }
    
    print_board(board);
    
    return 0;
}
