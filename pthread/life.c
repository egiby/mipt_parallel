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

int NUMBER_OF_STEPS = 0;
int NUMBER_OF_THREADS = 1;

void * recalc_part(void * args)
{
    void ** argv = (void**)args;
    Board * board = (Board*)argv[0];
    Board * new_board = (Board*)argv[0];
    int id = *(int*)argv[2];
    int n = board->N;
    int m = board->M;
    
    int l = calc_left(board->N, id, NUMBER_OF_THREADS);
    int r = calc_right(board->N, id, NUMBER_OF_THREADS);
    
    int * step_top = (int*)argv[3];
    int * step_top_to_read = (int*)argv[4];
    int * step_bottom = (int*)argv[5];
    int * step_bottom_to_read = (int*)argv[6];
    
    //~ int * bottom = malloc(sizeof(int) * board->M);
    //~ int * top = malloc(sizeof(int) * board->M);
    
    //~ int * old_top = malloc(sizeof(int) * board->M);
    //~ int * old_bottom = malloc(sizeof(int) * board->M);
    
    for (int k = 0; k < NUMBER_OF_STEPS; ++k)
    {
        //~ int_arr_copy(old_top, board->board[l], m);
        while (int_get_element(id - 1, n, step_bottom_to_read) < k)
            pthread_yield();
        
        for (int y = 0; y < m; ++y)
        {
            new_board[l][y] = ;
        }
        
        //~ step_top[id] = k + 1;
        
        //~ while (int_get_element(id - 1, n, step_bottom) < k)
    }
    
    return NULL;
}

void play(Board * board)
{
    pthread_t * t = malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);
    
    int * step_top = malloc(sizeof(int) * NUMBER_OF_THREADS);
    memset(step_top, 0, sizeof(int) * NUMBER_OF_THREADS);
    
    int * step_bottom = malloc(sizeof(int) * NUMBER_OF_THREADS);
    memset(step_bottom, 0, sizeof(int) * NUMBER_OF_THREADS);
    
    int * step_top_to_read = malloc(sizeof(int) * NUMBER_OF_THREADS);
    memset(step_top_to_read, 0, sizeof(int) * NUMBER_OF_THREADS);
    
    int * step_bottom_to_read = malloc(sizeof(int) * NUMBER_OF_THREADS);
    memset(step_bottom_to_read, 0, sizeof(int) * NUMBER_OF_THREADS);
    
    Board * new_board = get_board(board->N, board->M);
    
    for (int i = 0; i < NUMBER_OF_THREADS; ++i)
    {
        void ** argv = malloc(sizeof(void*) * 6);
        
        int * id = malloc(sizeof(int));
        *id = i;
        
        argv[0] = (void*)board;
        argv[1] = (void*)new_board;
        argv[2] = (void*)id;
        argv[3] = (void*)step_top;
        argv[4] = (void*)step_top_to_read;
        argv[5] = (void*)step_bottom;
        argv[6] = (void*)step_bottom_to_read;
        
        pthread_create(&t[i], NULL, recalc_part, argv);
    }
    
    for (int i = 0; i < NUMBER_OF_THREADS; ++i)
    {
        pthread_join(t[i], NULL);
    }
    
    free(t);
    free(step_bottom);
    free(step_top);
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
    
    //~ new_board = get_board(n, m);
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
