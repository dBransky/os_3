#include <stdio.h>
#include <pthread.h>

typedef struct Queue
{
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int queue_size;
    int max_size;
    int *items;
} Queue;
typedef Queue* QHandle;
QHandle create_queue(int max_size);
void enqueue(QHandle,int);
int dequeue(QHandle);
void free_queue(QHandle);