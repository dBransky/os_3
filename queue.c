#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
QHandle create_queue(int max_size)
{
    QHandle obj = malloc(sizeof(struct Queue));
    obj->items=malloc(max_size*(sizeof(int)));
    obj->max_size=max_size;
    obj->queue_size = 0;
    int cond_status = pthread_cond_init(&(obj->cond), NULL);
    int lock_status = pthread_mutex_init(&(obj->lock), NULL);
    return obj;
}
void enqueue(QHandle obj, int item)
{
    pthread_mutex_lock(&(obj->lock));
    printf("cond is %d\n",obj->cond);
    if(obj->queue_size==obj->max_size)
        //queue limit here prob return int
    obj->items[obj->queue_size]=item;
    (obj->queue_size)++;
    printf("signaling on lock %d\n",&(obj->lock));
    pthread_cond_signal(&(obj->cond));
    printf("cond is %d\n",obj->cond);
    pthread_mutex_unlock(&(obj->lock));
}
int dequeue(QHandle obj)
{
    pthread_mutex_lock(&(obj->lock));
    while ((obj->queue_size) == 0)
    {
        printf("waiting on lock %d\n",&(obj->lock));
        pthread_cond_wait(&(obj->cond), &(obj->lock));
        printf("wakeup thread");
    }
    (obj->queue_size)--;
    int item=obj->items[obj->queue_size];
    pthread_mutex_unlock(&(obj->lock));
    return item;
}
void free_queue(QHandle obj) { free(obj); }