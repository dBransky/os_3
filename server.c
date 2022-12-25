#include "segel.h"
#include "request.h"
#include "queue.h"

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
void *exe_job(QHandle req_queue)
{
    while (1)
    {
        int connfd = dequeue(req_queue);
        requestHandle(connfd);
        Close(connfd);
    }
}

pthread_t *init_threads(int threads, QHandle req_queue)
{
    pthread_t status;
    pthread_t *thread_tbl = malloc(threads * sizeof(int));
    for (int i = 0; i < threads; i++)
    {
        pthread_t thread;
        status = pthread_create(&thread, NULL, exe_job, req_queue);
        thread_tbl[i] = thread;
    }
    return thread_tbl;
}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads, queue_size;
    struct sockaddr_in clientaddr;
    char *schedalg = argv[4];
    getargs(&port, &threads, &queue_size, argc, argv);
    QHandle req_queue = create_queue(queue_size);
    pthread_t *thread_tbl;
    thread_tbl = init_threads(threads, req_queue);

    //
    // HW3: Create some threads...
    //

    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        enqueue(req_queue, connfd);
    }
    //
    // HW3: In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads
    // do the work.
    //
}
