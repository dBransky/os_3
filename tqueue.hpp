#include <pthread.h>
#include <queue>
template <class T>
struct TQueue
{
    pthread_cond_t cond;
    pthread_mutex_t lock;
    int queue_size;
    std::queue<T> queue;

public:
    TQueue()
    {   queue_size=0;
        int cond_status = pthread_cond_init(&(this->cond), NULL);
        int lock_status = pthread_mutex_init(&(this->lock), NULL);
    }
    void enqueue(T item)
    {
        pthread_mutex_lock(&lock);
        // add item to queue
        queue.push(item);
        queue_size++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
    T dequeue()
    {   
        pthread_mutex_lock(&lock);
        while(queue_size==0){
            pthread_cond_wait(&cond);
        }
        T item =queue.pop();
        queue_size--;
        pthread_mutex_unlock(&lock);
        return item;
    }
};