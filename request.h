#ifndef __REQUEST_H__
typedef struct Worker
{
    pthread_t thread_id;
    int id;
    int reqs_handled;
    int dynamic_reqs;
    int static_reqs;
} Worker;
typedef struct Request
{   int handler_thread;
    int connfd;
    struct timeval arive_time;
    struct timeval dispatch_interval;
} Request;
void requestHandle(Request req,Worker* thread_tbl);

#endif
