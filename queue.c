#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
void print_items(QHandle obj)
{
    for (int i = 0; i < obj->queue_size; i++)
    {
    }
}
void inset_item(QHandle obj, Request item)
{
    for (int i = obj->queue_size; i > 0; i--)
    {
        obj->items[i] = obj->items[i - 1];
    }
    (obj->items)[0] = item;
    (obj->queue_size)++;
}
void remove_randoms(QHandle obj)
{
    int replaced = 0;
    while (replaced != (((obj->queue_size)+1) / 2))
    {
        int rand_int = rand() % (obj->queue_size);
        if ((obj->items)[rand_int].connfd != -1)
        {   Request temp_req={-1,-1,-1};
            Close((obj->items)[rand_int].connfd);
            (obj->items)[rand_int] = temp_req;
            replaced++;
        }
    }
    Request arr_buff[obj->max_size];
    int buff_size = 0;
    for (int i = 0; i < obj->queue_size; i++)
    {
        if (obj->items[i].connfd != -1)
        {
            arr_buff[buff_size] = obj->items[i];
            buff_size++;
        }
    }
    obj->queue_size = obj->queue_size - (((obj->queue_size)+1) / 2);
    for (int i = 0; i < obj->queue_size; i++)
    {
        obj->items[i]=arr_buff[i];
    }
}
QHandle create_queue(int max_size, char *schedalg)
{
    QHandle obj = malloc(sizeof(struct Queue));
    obj->items = malloc(max_size * (sizeof(Request)));
    obj->schedalg = malloc(strlen(schedalg));
    strcpy(obj->schedalg, schedalg);
    obj->max_size = max_size;
    obj->queue_size = 0;
    obj->current_reqs = 0;
    int cond_status = pthread_cond_init(&(obj->not_empty), NULL);
    cond_status = pthread_cond_init(&(obj->not_full), NULL);
    int lock_status = pthread_mutex_init(&(obj->lock), NULL);
    return obj;
}
void enqueue(QHandle obj, Request item)
{
    pthread_mutex_lock(&(obj->lock));
    // queue limit here prob return int
    if (obj->queue_size+obj->current_reqs == obj->max_size)
    {
        if (strcmp("block", obj->schedalg) == 0)
        {
            while (obj->queue_size+obj->current_reqs == obj->max_size)
            {
                pthread_cond_wait(&(obj->not_full), &(obj->lock));
            }
        }
        if (strcmp("dt", obj->schedalg) == 0)
        {
            pthread_mutex_unlock(&(obj->lock));
            Close(item.connfd);
            return;
        }
        if (strcmp("dh", obj->schedalg) == 0)
        {   
            if(obj->queue_size==0){
                pthread_mutex_unlock(&(obj->lock));
                Close(item.connfd);
                return;
            }
            Close(((obj->items)[obj->queue_size-1]).connfd);
            (obj->queue_size)--;
        }
        if (strcmp("random", obj->schedalg) == 0)
        {
            if((((obj->queue_size)+1) / 2)==0){
                pthread_mutex_unlock(&(obj->lock));
                Close(item.connfd);
                return;
            }
            remove_randoms(obj);
        }
    }
    inset_item(obj, item);
    pthread_cond_signal(&(obj->not_empty));
    pthread_mutex_unlock(&(obj->lock));
}
Request dequeue(QHandle obj)
{
    pthread_mutex_lock(&(obj->lock));
    while ((obj->queue_size) == 0)
    {
        pthread_cond_wait(&(obj->not_empty), &(obj->lock));
    }
    (obj->current_reqs)++;
    (obj->queue_size)--;
    Request item = (obj->items)[obj->queue_size];
    pthread_mutex_unlock(&(obj->lock));
    return item;
}
void dec_current_reqs(QHandle obj){
    pthread_mutex_lock(&(obj->lock));
    (obj->current_reqs)--;
    pthread_cond_signal(&(obj->not_full));
    pthread_mutex_unlock(&(obj->lock));

}
void free_queue(QHandle obj) { free(obj->items), free(obj->schedalg), free(obj); }