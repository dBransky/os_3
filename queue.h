#ifndef __QUEUE_H__
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include "request.h"
typedef struct Queue
{
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    pthread_mutex_t lock;
    int queue_size;
    int max_size;
    int current_reqs;
    Request *items;
    char *schedalg;

} Queue;
typedef Queue *QHandle;
QHandle create_queue(int max_size, char *schedalg);
void enqueue(QHandle, Request);
Request dequeue(QHandle);
void free_queue(QHandle);
void dec_current_reqs(QHandle);
#endif