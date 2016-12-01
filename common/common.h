#ifndef _COMMON
#define _COMMON

int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
int dy[] = {1, 0, -1, 0, 1, -1, 1, -1};

void swap_int(int *x, int *y)
{
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
}

void swap_long(long *x, long *y)
{
    *x ^= *y;
    *y ^= *x;
    *x ^= *y;
}

void swap_iters(int ** a, int ** b)
{
    int * t = *a;
    *a = *b;
    *b = t;
}

void swap_iters2(int *** a, int *** b)
{
    int ** t = *a;
    *a = *b;
    *b = t;
}

int calc_left(int array_size, int thread_idx, int num_threads)
{
    return thread_idx * (array_size / num_threads);
}

int calc_right(int array_size, int thread_idx, int num_threads)
{
    return thread_idx != num_threads - 1 ? (thread_idx + 1) * (array_size / num_threads) : array_size;
}

int get_size(int array_size, int thread_idx, int num_threads)
{
    return calc_right(array_size, thread_idx, num_threads) - calc_left(array_size, thread_idx, num_threads);
}

int get_life_value(int cnt, int old_value)
{
    if (old_value == 1 && cnt != 2 && cnt != 3)
        return 0;
    else if (old_value == 1)
        return 1;
    
    if (cnt == 3)
        return 1;
    
    return 0;
}

int int_get_element(int id, int n, int * arr)
{
    return arr[(id + n) % n];
}

void int_arr_copy(int * dest, int * src, int n)
{
    for (int i = 0; i < n; ++i)
        dest[i] = src[i];
}

#endif
