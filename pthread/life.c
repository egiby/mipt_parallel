#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include "../common/board.h"
#include "../common/common.h"

int NUMBER_OF_STEPS = 0;
int NUMBER_OF_THREADS = 1;

void calc_line(int x, Board * board, Board * new_board)
{
    for (int y = 0; y < board->M; ++y)
        new_board->board[x][y] = get_life_value_by_board(x, y, board);
}

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void debug_print(const char * s)
{
    pthread_mutex_lock(&m);
    fprintf(stderr, s);
    pthread_mutex_unlock(&m);
}

void debug_int(int c)
{
    pthread_mutex_lock(&m);
    fprintf(stderr, "%d", c);
    pthread_mutex_unlock(&m);
}

void * recalc_part(void * arg)
{
    void ** argv = (void**)arg;
    Board * board = (Board*)argv[0];
    Board * new_board = (Board*)argv[1];
    int id = *(int*)argv[2];
    int m = board->M;
    
    int l = calc_left(board->N, id, NUMBER_OF_THREADS);
    int r = calc_right(board->N, id, NUMBER_OF_THREADS);
    
    int * step_top = (int*)argv[3];
    int * step_top_ready = (int*)argv[4];
    int * step_bottom = (int*)argv[5];
    int * step_bottom_ready = (int*)argv[6];
    
    int top_thread = (id - 1 + NUMBER_OF_THREADS) % NUMBER_OF_THREADS;
    int bottom_thread = (id + 1) % NUMBER_OF_THREADS;
    
    for (int k = 0; k < NUMBER_OF_STEPS; ++k)
    {
        while (step_bottom[top_thread] < k)
            sched_yield();
        
        calc_line(l, board, new_board);
        step_top_ready[id] = k + 1;
        
        calc_line(l + 1, board, new_board);
        
        while (step_top[bottom_thread] < k)
            sched_yield();
        
        calc_line(r - 1, board, new_board);
        step_bottom_ready[id] = k + 1;
        
        calc_line(r - 2, board, new_board);
        
        while (step_bottom_ready[top_thread] < k + 1)
            sched_yield();
        swap_iters(&new_board->board[l], &board->board[l]);
        step_top[id] = k + 1;
        
        while (step_top_ready[bottom_thread] < k + 1)
            sched_yield();
        swap_iters(&(new_board->board[r - 1]), &(board->board[r - 1]));
        step_bottom[id] = k + 1;
        
        for (int x = l + 2; x < r - 2; ++x)
            for (int y = 0; y < m; ++y)
                new_board->board[x][y] = get_life_value_by_board(x, y, board);
        
        for (int x = l + 1; x < r - 1; ++x)
            swap_iters(&(new_board->board[x]), &(board->board[x]));
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
