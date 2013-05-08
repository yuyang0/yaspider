#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "statistic.h"
#include "log.h"
#include "str.h"

#define BUFSIZE 4096

int Writen(int fd, void *ptr, size_t nbytes);
static int write_str(int fd, char *str);
static int write_int(int fd, int i);
static void write_to_cli(int connfd);
static int parse_request(char *buf, int data_size);
static void process_request(int connfd);

static time_t start_time;

void *start_server(void *p_port)
{
    /* get the time when the server is started*/
    start_time = time(NULL);
    int *p = p_port;
	int port = *p;
    int optval;
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		log_error("can not start local server");
		return ((void *)-1);
	}
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int))<0)
    {
        return ((void *)-2);
    }
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0 || listen(listenfd, 10) != 0)
	{
		close(listenfd);
		log_error("bind or listen error");
		return ((void *)-1);
	}
	for (; ; )
	{
		clilen = sizeof(cliaddr);
		int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		if (connfd < 0)
		{
			log_error("web server:an error happens and the server will stop");
			return ((void *)-1);
		}
		process_request (connfd);
	}
	return ((void *)0);
}
void process_request(int connfd)
{
	char buf[BUFSIZE];
	int pos = 0;
	int parse_ret = -1;
	while(1)
	{
	    int n =read(connfd, buf + pos, BUFSIZE - pos);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                break;
            }
        }
        else
        {
            pos += n;
		    buf[pos] = '\0';
            if (strstr(buf, "\r\n"))
            {
                parse_ret = parse_request(buf, pos);
                break;
            }
        }
	}
    if (parse_ret < 0)
    {
        close(connfd);
        return;
    }
    write_to_cli(connfd);
    close (connfd);
}
static int parse_request(char *buf, int data_size)
{
	if (startscasewith(buf, "GET / "))
	{
		return 0;
	}
	else
	{
        fprintf(stderr, "not get home page");
		return -1;
	}
}
void write_to_cli(int connfd)
{
    time_t now = time(NULL);
    write_str(connfd, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nServer: Jetty\r\n\r\n");
    write_str(connfd, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">");
	write_str(connfd, "<html><head><title>The status of Yaspider</title></head><body><h1>The Status of Yaspider</h1>");
    write_str(connfd, "<table cellpadding=\"3\" border=\"0\">");
    write_str(connfd, "<tr><td>ram urls:</td><td>");
    write_int(connfd, statis.ram_urls);
    write_str(connfd, "</td></tr><tr><td>ok urls:</td><td>");
    write_int(connfd, statis.ok_urls);
    int speed = statis.ok_urls / (now - start_time);
    write_str(connfd, "</td></tr><tr><td>ok urls speed(per second):</td><td>");
    write_int(connfd, speed);
    write_str(connfd, "</td></tr><tr><td>error urls:</td><td>");
    write_int(connfd, statis.error_urls);
    write_str(connfd, "</td></tr><tr><td>skipped urls:</td><td>");
    write_int(connfd, statis.skipped_urls);
    
    write_str(connfd, "</td></tr><tr><td>ram sites:</td><td>");
    write_int(connfd, statis.ram_sites);
    write_str(connfd, "</td></tr><tr><td>dns ok sites:</td><td>");
    write_int(connfd, statis.dns_ok_sites);
    write_str(connfd, "</td></tr><tr><td>dns error sites:</td><td>");
    write_int(connfd, statis.dns_err_sites);

    write_str(connfd, "</td></tr></table><hr></body></html>");
}
static int write_str(int fd, char *str)
{
	size_t len = strlen(str);
	return Writen(fd, str, len);
}
static int write_int(int fd, int i)
{
	char buf[30];
	/*fixme:snprintf*/
	sprintf(buf, "%d", i);
	return Writen(fd, buf, strlen(buf));
}
typedef void *(*THREAD_FUNC)(void *arg);

int start_thread(THREAD_FUNC fn, void *arg)
{
	pthread_t ntid;
	pthread_attr_t attr;
	int err = pthread_attr_init(&attr);
	if (err != 0)
		return err;
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err == 0)
		err = pthread_create(&ntid, &attr, fn, arg);
	pthread_attr_destroy(&attr);
	return(err);
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
    {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */


int Writen(int fd, void *ptr, size_t nbytes)
{
	if (writen(fd, ptr, nbytes) != nbytes)
	{
		perror("writen error");
		return -1;
	}
	return nbytes;
}


/*  debug  */
#if 0
struct statistic statis = {1, 2, 3, 4, 5, 6, 7, 8, 9};
int main(int argc, char *argv[])
{
    static int port = 1111;
    /* start_thread(start_server, &port); */
    /* int i = 0; */
    /* for (; ;) */
    /* { */
    /*     i++; */
    /* } */
    start_server(&port);
    return 0;
}
#endif
