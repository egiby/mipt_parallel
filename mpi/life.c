#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <mpi.h>

#include "../common/board.h"
#include "../common/common.h"

int * get_line_board(Board * board, int thread_idx, int num_threads)
{
    int size = get_size(board->N, thread_idx, num_threads);
    int l = calc_left(board->N, thread_idx, num_threads);
    int * line_board = malloc(size * board->M * sizeof(int));
    
    for (int i = 0; i < size; ++i)
        memcpy(line_board + i * board->M, board->board[i + l], board->M * sizeof(int));
    
    return line_board;
}

Board * get_board_from_line(int * line_board, int n, int m)
{
    Board * board = get_board(n, m);
    
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            board->board[i][j] = line_board[i * m + j];
    
    return board;
}

int get_line_board_elem(int * board, int * top, int * bottom, int x, int y, int size, int m)
{
    int ans = -1;
    y = (y + m) % m;
    
    if (x == -1)
        ans = top[y];
    else if (x == size)
        ans = bottom[y];
    else
        ans = board[x * m + y];
    
    return ans;
}

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);
    
    int num_threads = -1;
    MPI_Comm_size(MPI_COMM_WORLD, &num_threads);
    
    int thread_idx = -1;
    MPI_Comm_rank(MPI_COMM_WORLD, &thread_idx);
    
    int *thread_board;
    
    long n, m, k;
    
    int size;
    char is_reversed = 0;
    
    // reading
    if (thread_idx == 0)
    {
        FILE * file = fopen("board", "r");
        fscanf(file, "%ld %ld %ld\n", &n, &m, &k);
        
        Board * board = get_board(n, m);
        
        read_board(board, file);
        
        if (n < m)
        {
            reverse_board(board);
            swap_long(&n, &m);
            is_reversed = 1;
        }
        
        size = get_size(n, thread_idx, num_threads);
        for (int i = 1; i < num_threads; ++i)
        {
            //~ printf("%d\n", i);
            int size_i = get_size(n, i, num_threads);
            int * line_board = get_line_board(board, i, num_threads);
            
            MPI_Send(&n, 1, MPI_LONG, i, 50000, MPI_COMM_WORLD);
            MPI_Send(&m, 1, MPI_LONG, i, 50001, MPI_COMM_WORLD);
            MPI_Send(&k, 1, MPI_LONG, i, 50002, MPI_COMM_WORLD);
            MPI_Send(&size_i, 1, MPI_INT, i, 50003, MPI_COMM_WORLD);
            MPI_Send(line_board, size_i * m, MPI_INT, i, 50004, MPI_COMM_WORLD);
            
            free(line_board);
        }
        
        thread_board = get_line_board(board, 0, num_threads);
        
        print_board(board);
        
        delete_board(board);
    }
    else
    {
        MPI_Recv(&n, 1, MPI_LONG, 0, 50000, MPI_COMM_WORLD, NULL);
        MPI_Recv(&m, 1, MPI_LONG, 0, 50001, MPI_COMM_WORLD, NULL);
        MPI_Recv(&k, 1, MPI_LONG, 0, 50002, MPI_COMM_WORLD, NULL);
        MPI_Recv(&size, 1, MPI_INT, 0, 50003, MPI_COMM_WORLD, NULL);
        thread_board = malloc(size * m * sizeof(int));
        MPI_Recv(thread_board, size * m, MPI_INT, 0, 50004, MPI_COMM_WORLD, NULL);
    }
    
    int * top = malloc(m * sizeof(int));
    int * bottom = malloc(m * sizeof(int));
    
    int * new_board = malloc(size * m * sizeof(int));
    
    for (int i = 0; i < k; ++i)
    {
        int left_thread = (thread_idx - 1 + num_threads) % num_threads;
        int right_thread = (thread_idx + 1) % num_threads;
        
        MPI_Send(thread_board, m, MPI_INT, left_thread, 2 * i, MPI_COMM_WORLD);
        MPI_Send(thread_board + (size - 1) * m, m, MPI_INT, right_thread, 2 * i + 1, MPI_COMM_WORLD);
        
        MPI_Recv(top, m, MPI_INT, left_thread, 2 * i + 1, MPI_COMM_WORLD, NULL);
        MPI_Recv(bottom, m, MPI_INT, right_thread, 2 * i, MPI_COMM_WORLD, NULL);
        
        for (int x = 0; x < size; ++x)
            for (int y = 0; y < m; ++y)
            {
                int old_value = get_line_board_elem(thread_board, top, bottom, x, y, size, m);
                int cnt = 0;
                
                for (int l = 0; l < 8; ++l)
                    cnt += get_line_board_elem(thread_board, top, bottom, x + dx[l], y + dy[l], size, m);
                
                new_board[x * m + y] = get_life_value(cnt, old_value);
            }
        
        swap_iters(&new_board, &thread_board);
    }
    
    int * line_board = NULL;
    int * count_r = NULL;
    int * disp_r = NULL;
    
    if (thread_idx == 0)
    {
        line_board = malloc(sizeof(int) * n * m);
        count_r = malloc(sizeof(int) * num_threads);
        disp_r = malloc(sizeof(int) * num_threads);
        
        int last = 0;
        for (int i = 0; i < num_threads; ++i)
        {
            disp_r[i] = last;
            count_r[i] = get_size(n, i, num_threads) * m;
            last += count_r[i];
        }
    }
    
    MPI_Gatherv(thread_board, size * m, MPI_INT, line_board, count_r, disp_r, 
                MPI_INT, 0, MPI_COMM_WORLD);
    
    if (thread_idx == 0)
    {
        memcpy(line_board, thread_board, m * size * sizeof(int));
        Board * board = get_board_from_line(line_board, n, m);
        
        if (is_reversed)
            reverse_board(board);
        
        print_board(board);
        
        delete_board(board);
        free(line_board);
        free(count_r);
        free(disp_r);
    }
    
    free(thread_board);
    free(new_board);
    
    MPI_Finalize();
    return 0;
}
