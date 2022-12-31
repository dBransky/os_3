#include "segel.h"
#include "queue.h"
#include <stdio.h>
#include <sys/time.h>

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
Worker *thread_tbl;
void getargs(int *port, int *threads, int *queue_size, int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
}
typedef struct Args{
    QHandle req_queue;
    int id;
}Args;
void *exe_job(Args* args)
{
    QHandle req_queue=args->req_queue;
    while (1)
    {
        Request req = dequeue(req_queue);
        // maybe check return val???
        req.handler_thread=args->id;
        struct timeval currrent_time;
        gettimeofday(&currrent_time, NULL);
        timersub(&currrent_time,&req.arive_time,&req.dispatch_interval);
        requestHandle(req, thread_tbl);
        dec_current_reqs(req_queue);
        Close(req.connfd);
    }
}

void init_threads(int threads, QHandle req_queue)
{
    pthread_t status;
    thread_tbl = malloc(threads * sizeof(Worker));
    for (int i = 0; i < threads; i++)
    {
        pthread_t thread;
        Args* args=malloc(sizeof(Args));
        args->id=i;
        args->req_queue=req_queue;
        status = pthread_create(&thread, NULL, exe_job, args);
        Worker worker = {thread, i, 0, 0, 0};
        thread_tbl[i] = worker;
    }
    return thread_tbl;
}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads, queue_size;
    struct sockaddr_in clientaddr;
    char *schedalg = argv[4];
    getargs(&port, &threads, &queue_size, argc, argv);
    QHandle req_queue = create_queue(queue_size, schedalg);
    init_threads(threads, req_queue);

    //
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        Request req;
        gettimeofday(&req.arive_time, NULL);
        req.connfd = connfd;
        enqueue(req_queue, req);
    }
    //
    // HW3: In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads
    // do the work.
    //
}
