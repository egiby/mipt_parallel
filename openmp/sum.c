#include <stdio.h>
#include <omp.h>

#include "../common/common.h"

#define MAXN 1000
int a[MAXN];

int main()
{
    long long s = 0;
    
    int n = 0;
    scanf("%d", &n);
    
    for (int i = 0; i < n; ++i)
        scanf("%d", &a[i]);
    
#pragma omp parallel reduction (+:s)
    {
        int left = calc_left(n, omp_get_thread_num(), omp_get_num_threads());
        int right = calc_right(n, omp_get_thread_num(), omp_get_num_threads());
        
        for (int i = left; i < right; ++i)
            s += a[i];
    }
    
    printf("%lld\n", s);
    return 0;
}
