/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  init_ev.c
 *        Created:  2013-03-19 22:03:17
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/dns.h>

#include <sys/socket.h>
#include <time.h>
#include <errno.h>

#include "core.h"

#define URLS_PER_FETCH 100

#define DEBUG 1

static fifo_t *ready_sites_fifo;
static fifo_t *wait_sites_fifo;

/********** private functios*********************/
static site_t *ready_fifo_get();
static void ready_fifo_put(site_t *si);
static size_t ready_fifo_num();
static site_t *wait_fifo_get();
static void wait_fifo_put(site_t *si);
static size_t wait_fifo_num();

static int create_http_req(url_t *u, char *buf);

static bool is_html(conn_t *cn);
/****************end*****************************/
void fetch_urls();
void fetch_dns();
void fetch_pages();

static struct event_base *main_base;
static struct evdns_base *dns_base;

void event_cb(struct bufferevent *bev, short events, void *ptr)
{
    conn_t *cn = (conn_t *) ptr;
    url_t *u = cn->url;
    bool finished = false;
    if (events & BEV_EVENT_CONNECTED)
    {
        /* log_info("connected:%s", u->full_url); */
        /*start writing*/
        char buffer[BUFSIZE] = {0};
        create_http_req(u, buffer);
        //struct evbuffer *output = bufferevent_get_output(bev);
        bufferevent_write(bev, buffer, strlen(buffer));
    }
    if (events & BEV_EVENT_ERROR)
    {
        finished = true;
        log_error("bufferevent error:%s, %s", u->full_url, evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        statis.error_urls++;
    }
    if (events & BEV_EVENT_EOF)
    {
        finished = true;
        char buf[BUFSIZE];
        int n;
        struct evbuffer *input = bufferevent_get_input(bev);
        while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
        {
            conn_append(cn, buf, n);
        }
        /*
          parse the buffer we read from the bufferevent
          and init the connection's resp field
        */
        /* log_debug("http_resp_init: %s, %s", cn->url->full_url, cn->buf);*/
        if (http_resp_init(cn->resp, cn->buf, cn->dsize) < 0)
        {
            log_error("http response parse error: %s", cn->url->full_url);
        }
        else
        {
            // /* is robots.txt */
            // if (!cn->site->robots_gotten)
            // {
            //     parse_robots(cn);
            //     cn->site->robots_gotten = true;
            // }
            // else
            // {
            switch(cn->resp->status_code / 100)
            {
            case 3:
            {
                char *location = http_hdr_list_get_value(cn->resp->headers, "Location");
                if (location)
                {
                    put_url_str(location, 0);
                }
            }
            break;
            case 2:
                /* is a html? parse it and store all urls to url fifo*/
                if (is_html(cn))
                {
                    fetch_all_urls(cn);
                }
                /* save the content to file*/
                save(cn);
                /*update the statistics*/
                statis.ok_urls++;
                log_ok_url("%s", cn->url->full_url);
                break;
            case 4:
                log_error_url("%s", cn->url->full_url);
                statis.error_urls++;
                break;
            default:
                break;
            }
            //}
        }
    }
    if (finished)
    {
        bufferevent_free(bev);
        stop_conn(cn);

        statis.conns--;
        statis.ram_urls--;

        fetch_urls();
        fetch_dns();
        fetch_pages();
    }
}

void read_cb(struct bufferevent *bev, void *ptr)
{
    conn_t *cn = (conn_t *) ptr;
    char buf[BUFSIZE];
    int n;
    struct evbuffer *input = bufferevent_get_input(bev);
    while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0)
    {
        conn_append(cn, buf, n);
    }
}

void dns_cb(int errcode, struct evutil_addrinfo *addr, void *ptr)
{
    site_t *si = (site_t *) ptr;
    
    const char *host = si->host;
    if (errcode)
    {
        si->dns_state = ERROR_DNS;
        statis.dns_err_sites++;
        log_error("%s -> %s\n", host, evutil_gai_strerror(errcode));
    }
    else
    {
        struct evutil_addrinfo *ai;
        if (addr->ai_canonname)
            si->cname = strdup(addr->ai_canonname);
        int i = 0;
        for (ai = addr; ai; ai = ai->ai_next)
        {
            if (ai->ai_family == AF_INET)
            {
            	if (i < MAX_ADDRS)
            	{
            		si->addr[i++] = *((struct sockaddr_in *)ai->ai_addr);
            	}
            }
        }
        evutil_freeaddrinfo(addr);
        if(i > 0)
        {
            si->dns_state = DONE_DNS;
            si->next_call = (time_t)0;
            statis.dns_ok_sites++;
            if (!site_empty(si) && !si->in_ready_fifo)
            {
                ready_fifo_put(si);
            }
        }
    }
    statis.dns_calls--;

    //fetch_urls();
    fetch_dns();
    fetch_pages();
}
void init_ev()
{
    main_base = event_base_new();
    dns_base = evdns_base_new(main_base, 1);
    
    ready_sites_fifo = fifo_new(NULL);
    wait_sites_fifo = fifo_new(NULL);
}
int create_bufferev (conn_t *cn)
{
    struct bufferevent *bev;
    bev = bufferevent_socket_new(main_base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, event_cb, cn);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
    
    struct sockaddr_in sin = cn->site->addr[0];
    sin.sin_port = htons(cn->url->port);
    
    if (bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        /* Error starting connection */
        bufferevent_free(bev);
        stop_conn(cn);
        log_error("create_bufferev: error starting connection");
        statis.ram_urls--;
        return -1;
    }
    /*update the number of connections*/
    statis.conns++;
    
    return 0;
}
void create_dns(site_t *si)
{
    assert(si->dns_state == WAIT_DNS);
    assert(!si->in_ready_fifo && !si->in_wait_fifo);
    
    struct evutil_addrinfo hints;
    struct evdns_getaddrinfo_request *req;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = EVUTIL_AI_CANONNAME;
    /* Unless we specify a socktype, we'll get at least two entries for
     * each address: one for TCP and one for UDP. That's not what we
     * want. */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    req = evdns_getaddrinfo(
        dns_base, si->host, NULL /* no service name given */,
        &hints, dns_cb, si);
    if (req == NULL)
    {
        log_warn("  [request for %s returned immediately]\n", si->host);
        return;
    }
    /* update the  number of dns calls*/
    statis.dns_calls++;
}
void main_loop()
{
    event_base_loop(main_base, EVLOOP_NO_EXIT_ON_EMPTY);
}
static int create_http_req(url_t *u, char *buf)
{
    /*the string for GET, format: path[?query_string]*/
    char get_str[BUFSIZE];
    strcpy(get_str, u->path);
    if (u->query_str)
    {
        strcpy(get_str + strlen(get_str), "?");
        strcpy(get_str + strlen(get_str), u->query_str);
    }

    snprintf(buf, BUFSIZE, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\nAccept: text/html,application/xhtml+xml\r\nAccept-Encoding: gzip,deflate\r\n\r\n",
             get_str, u->host, settings.user_agent);
    return 0;
}

void fetch_urls()
{
    int i;
    int nr_prior_urls = prior_url_num();
    int nr_normal_urls = normal_url_num();
    int priority = 0;
    for (i = 0; i < URLS_PER_FETCH; ++i)
    {
        if (statis.ram_urls >= settings.max_ram_urls)
            return;
        /* get the priority */
        if (nr_prior_urls > 0)
        {
            priority = 1;
            nr_prior_urls--;
        }
        else if (nr_normal_urls > 0)
        {
            priority = 0;
            nr_normal_urls--;
        }
        else
        {
            return;
        }
        char *str = get_url_str(priority);
        if (!str)
        {
            return;
        }
        char *host = get_host(str);
        site_t *si = siteht_get_site(host);
        /*create a new site*/
        if(!si)
        {
            si = site_new();
            siteht_set(host, si);
            statis.ram_sites++;
        }
        /* dns error, skip the url*/
        if (ERROR_DNS == si->dns_state)
        {
            log_skipped_url("DNS error: %s", str);
            free(str);
            statis.skipped_urls++;
            continue;
        }
        /*
          enough urls in this site? reput this url string to url fifo
        */
        if(si->ram_urls >= settings.max_site_urls)
        {
            put_url_str(str, priority);
            free(str);
            continue;
        }
        url_t *u = url_new_with_str(str);
        free(str);
        site_put_url(si, u);
        statis.ram_urls++;
        /*update fifos*/
        if ((si->dns_state == DONE_DNS) &&
            (!si->in_ready_fifo))
        {
            ready_fifo_put (si);
        }
        
        if ((si->dns_state == WAIT_DNS) &&
            (!si->in_wait_fifo))
        {
            wait_fifo_put(si);
        }
    }
}

void fetch_dns()
{
    int space = settings.max_dns_calls - statis.dns_calls;
    int i = 0;
    while(i < space)
    {
        site_t *si = wait_fifo_get();
        if (!si)
            break;
        if (si->dns_state == WAIT_DNS)
        {
            create_dns(si);
            ++i;
        }
        else
        {
            continue;
        }
    }
}

void fetch_pages()
{
    time_t now = time(NULL);
    /* the number of elements in ready sites fifo */
    int nr_elements = ready_fifo_num();
    /* the number of free connections */
    int space = settings.max_conns - statis.conns;
    int i;
    for (i = 0; i < space && nr_elements > 0; ++i, nr_elements--)
    {
        site_t *si = ready_fifo_get ();
        if (!si)
            break;
        /*TODO: check site's time interval*/
        if (now <= si->next_call)
        {
            ready_fifo_put(si);
            continue;
        }
        url_t *u = site_get_url(si);
        // if (si->robots_gotten)
        // {
        //     while(1)
        //     {
        //         u = site_get_url(si);
        //         /* check if the url is forbidden by robots.txt */
        //         if (site_test_robots(si, u->path))
        //         {
        //             break;
        //         }
        //         else
        //         {
        //             url_destroy(u);
        //         }
        //     }
        //     /* all the urls are forbidden by robots.txt
        //        so ignore this site and goto next loop
        //      */
        //     if (!u)
        //     {
        //         continue;
        //     }
        // }
        // else
        // {
        //     u = url_new_with_host("http", si->host, "/robots.txt");
        // }
        conn_t *cn = get_free_conn();
        assert(cn);
        cn->site = si;
        cn->url = u;
        create_bufferev(cn);
        /* update the next time when we can connect to this server*/
        cn->site->next_call = now + settings.interval;
        // if (!cn->site->robots_gotten)
        // {
        //     cn->site->next_call = now + settings.interval;
        // }
        // else
        // {
        //     cn->site->next_call = 0;
        // }
        
        if (!site_empty(cn->site))
        {
            ready_fifo_put(cn->site);
        }
    }
}

/* functions for site fifo*/
static site_t *ready_fifo_get()
{
    site_t *si =  fifo_get(ready_sites_fifo);
    if (si)
    {
        si->in_ready_fifo = false;
    }
    return si;
}
static void ready_fifo_put(site_t *si)
{
    assert(si);
//    assert(!si->in_ready_fifo && !si->in_wait_fifo);
    if (fifo_put(ready_sites_fifo, si) < 0)
    {
        log_error("ready_fifo_put: fifo_put error (%s)", si->host);
        return;
    }
    else
    {
        si->in_ready_fifo = true;
    }
}
static size_t ready_fifo_num()
{
    return fifo_num(ready_sites_fifo);
}
static site_t *wait_fifo_get()
{
    site_t *si = fifo_get(wait_sites_fifo);
    if (si)
    {
        si->in_wait_fifo = false;
    }
    return si;
}
static void wait_fifo_put(site_t *si)
{
    assert(si);
    //assert(!si->in_wait_fifo && !si->in_ready_fifo);

    if (fifo_put(wait_sites_fifo, si) < 0)
    {
        log_error("wait_fifo_put: fifo_put error (%s)", si->host);
        return;
    }
    else
    {
        si->in_wait_fifo = true;
    }
}
static size_t wait_fifo_num()
{
    return fifo_num(wait_sites_fifo);
}
/*check the if the content of the http response is a html*/
static bool is_html(conn_t *cn)
{
	http_resp_t *resp = cn->resp;
	char *val = http_hdr_list_get_value(resp->headers, "Content-Type");
	if (val && strcasestr(val, "text/html"))
	{
		return true;
	}
	else
	{
		return false;
	}
}

char *get_host(char *url_str)
{
    static char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    strncpy(buf, url_str, MAXLINE - 1);

    char *q_str = strchr(buf, '?');
    if(q_str)
        *q_str = '\0';
    char *host = strstr(buf, "://");
    if (!host)
    {
        host = buf;
    }
    else
    {
        host += 3;  /*ignore ://  */
    }

    char *port = strchr (host, ':');
    char *path = strchr (host, '/');
	if (port)
    {
        *port = '\0';
    }
    else
    {
        if (path)
            *path = '\0';
    }
    return host;
}
