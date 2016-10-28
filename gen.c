#include <stdio.h>

#include "common/test.h"

int main(int argc, char ** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Must be thee argument: n, m, k\n");
        return 0;
    }
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int k = atoi(argv[3]);
    Board * board = get_rand_board(n, m);
    
    printf("%d %d %d\n", n, m, k);
    print_board(board);
    
    delete_board(board);
    return 0;
}
