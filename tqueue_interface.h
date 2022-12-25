struct TQueue;

#ifdef __cplusplus
extern "C" {
#endif

typedef TQueue* QHandle;
QHandle create_tqueue();
void enqueue_tqueue(QHandle,void *item);
void * dequeue_tqueue(QHandle);
void free_tqueue(QHandle);
#ifdef __cplusplus
}
#endif