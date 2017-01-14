#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <omp.h>

#include "../common/board.h"
#include "../common/test.h"
#include "../common/common.h"

int main(int argc, char ** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "It must be board's filename and number of threads\n");
        return 0;
    }
    FILE * file = fopen(argv[1], "r");
    int num_threads = atoi(argv[2]);
    
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
    
    Board * new_board = get_board(n, m);
    for (int i = 0; i < k; ++i)
#pragma omp parallel num_threads(num_threads)
    {
    #pragma omp barrier
        int left = calc_left(n, omp_get_thread_num(), omp_get_num_threads());
        int right = calc_right(n, omp_get_thread_num(), omp_get_num_threads());
        
        for (int x = left; x < right; ++x)
            for (int y = 0; y < m; ++y)
            {
                int cnt = 0;
                for (int j = 0; j < 8; ++j)
                    cnt += get_elem(board, x + dx[j], y + dy[j]);
                
                new_board->board[x][y] = get_life_value(cnt, get_elem(board, x, y));
            }
    #pragma omp barrier
    #pragma omp single
        swap_iters2(&new_board->board, &board->board);
    }
    
    if (is_reversed)
    {
        reverse_board(board);
        swap_int(&n, &m);
    }
    
    print_board(board);
    
    delete_board(board);
    delete_board(new_board);
    return 0;
}
