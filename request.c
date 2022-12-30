//
// request.c: Does the bulk of the work for the web server.
//

#include "segel.h"
#include "request.h"
#include <sys/wait.h>
void update_thread_tbl(Worker *thread_tbl, int id, int is_static)
{
   thread_tbl[id].reqs_handled++;
   if (is_static == 1)
      thread_tbl[id].static_reqs++;
   if (is_static == 0)
      thread_tbl[id].dynamic_reqs++;
}
// requestError(      fd,    filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(Request req, Worker *thread_tbl, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor="
                 "fffff"
                 ">\r\n",
           body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Req-Arrival:: %lu.%06lu\r\n", req.arive_time.tv_sec, req.arive_time.tv_usec);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Req-Dispatch:: %lu.%06lu\r\n", req.dispatch_interval.tv_sec, req.dispatch_interval.tv_usec);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Thread-Id:: %d\r\n", req.handler_thread);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Thread-Count:: %d\r\n", thread_tbl[req.handler_thread].reqs_handled);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Thread-Static:: %d\r\n", thread_tbl[req.handler_thread].static_reqs);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);
   sprintf(buf, "Stat-Thread-Dynamic:: %d\r\n\r\n", thread_tbl[req.handler_thread].dynamic_reqs);
   Rio_writen(req.connfd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(req.connfd, body, strlen(body));
   printf("%s", body);
}

//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n"))
   {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs)
{
   char *ptr;

   if (strstr(uri, ".."))
   {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi"))
   {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri) - 1] == '/')
      {
         strcat(filename, "home.html");
      }
      return 1;
   }
   else
   {
      // dynamic
      ptr = index(uri, '?');
      if (ptr)
      {
         strcpy(cgiargs, ptr + 1);
         *ptr = '\0';
      }
      else
      {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html"))
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif"))
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg"))
      strcpy(filetype, "image/jpeg");
   else
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(Request req, Worker *thread_tbl, char *filename, char *cgiargs)
{
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, req.arive_time.tv_sec, req.arive_time.tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, req.dispatch_interval.tv_sec, req.dispatch_interval.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, req.handler_thread);
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, thread_tbl[req.handler_thread].reqs_handled);
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, thread_tbl[req.handler_thread].static_reqs);
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, thread_tbl[req.handler_thread].dynamic_reqs);
   Rio_writen(req.connfd, buf, strlen(buf));

   printf("%d started wait\n", req.handler_thread);
   pid_t pid = Fork();
   if (pid == 0)
   {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(req.connfd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(pid, NULL, 0);
   printf("%d fnished wait\n", req.handler_thread);
}

void requestServeStatic(Request req, Worker *thread_tbl, char *filename, int filesize)
{
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory,
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, req.arive_time.tv_sec, req.arive_time.tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, req.dispatch_interval.tv_sec, req.dispatch_interval.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, req.handler_thread);
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, thread_tbl[req.handler_thread].reqs_handled);
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, thread_tbl[req.handler_thread].static_reqs);
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, thread_tbl[req.handler_thread].dynamic_reqs);
   Rio_writen(req.connfd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file
   Rio_writen(req.connfd, srcp, filesize);
   Munmap(srcp, filesize);
}

// handle a request
void requestHandle(Request req, Worker *thread_tbl)
{
   int fd = req.connfd;
   int is_static;
   struct stat sbuf;
   char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   rio_t rio;

   Rio_readinitb(&rio, fd);
   Rio_readlineb(&rio, buf, MAXLINE);
   sscanf(buf, "%s %s %s", method, uri, version);

   printf("%s %s %s\n", method, uri, version);

   if (strcasecmp(method, "GET"))
   {
      update_thread_tbl(thread_tbl, req.handler_thread, -1);
      requestError(req, thread_tbl, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method");
      return;
   }
   requestReadhdrs(&rio);

   is_static = requestParseURI(uri, filename, cgiargs);
   if (stat(filename, &sbuf) < 0)
   {
      update_thread_tbl(thread_tbl, req.handler_thread, -1);
      requestError(req, thread_tbl, filename, "404", "Not found", "OS-HW3 Server could not find this file");
      return;
   }

   if (is_static)
   {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
      {
         update_thread_tbl(thread_tbl, req.handler_thread, -1);
         requestError(req, thread_tbl, filename, "403", "Forbidden", "OS-HW3 Server could not read this file");
         return;
      }
      update_thread_tbl(thread_tbl, req.handler_thread, 1);
      requestServeStatic(req, thread_tbl, filename, sbuf.st_size);
   }
   else
   {
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
      {
         update_thread_tbl(thread_tbl, req.handler_thread, -1);
         requestError(req, thread_tbl, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program");
         return;
      }
      update_thread_tbl(thread_tbl, req.handler_thread, 0);
      requestServeDynamic(req, thread_tbl, filename, cgiargs);
   }
}
