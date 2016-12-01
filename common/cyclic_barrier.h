#ifndef _CYCLIC_BARRIER
#define _CYCLIC_BARRIER

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

typedef struct CyclicBarrier
{
    int count;
    int max_count;
    pthread_mutex_t mutex;
    sem_t * sem;
    void (*f)(void **);
} CyclicBarrier;


CyclicBarrier * get_cyclic_barrier(int n, void (*f)(void **))
{
    CyclicBarrier * barrier = (CyclicBarrier *)malloc(sizeof(CyclicBarrier));
    pthread_mutex_init(&barrier->mutex, NULL);
    
    barrier->sem = malloc(sizeof(sem_t));
    sem_init(barrier->sem, 0, 0);
    
    barrier->count = 0;
    barrier->max_count = n;
    
    barrier->f = f;
    return barrier;
}


void lock_barrier(CyclicBarrier * barrier, void ** argv)
{
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;
    
    if (barrier->count == barrier->max_count)
    {
        barrier->f(argv);
        
        for (int i = 0; i < barrier->max_count; ++i)
            sem_post(barrier->sem);
        
        barrier->count = 0;
    }
    
    pthread_mutex_unlock(&barrier->mutex);
    
    sem_wait(barrier->sem);
}
#endif
