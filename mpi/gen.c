#include <stdio.h>

#include "../common/test.h"

int main()
{
    Board * board = get_rand_board(1000, 10000);
    
    printf("%d %d %d\n", 1000, 10000, 100);
    print_board(board);
    
    delete_board(board);
    return 0;
}
