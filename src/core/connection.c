/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  connection.c
 *        Created:  2013-04-09 12:43:25
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include "core.h"

#define NR_CONNS settings.max_conns
#define INIT_SIZE 16384         /*the initial size of connection buffer: 16 KB */
struct conn_head_s
{
    queue_t free_sentinel;     /*the sentinel for free connection list*/
	queue_t used_sentinel;
	conn_t *conns;             /*the connection array(we need this field when free the memory)*/
	size_t nr_free;
	size_t nr_used;
};
static struct conn_head_s global_chead;   //global conn_head

static int init_global_chead ();
static int conn_init(conn_t *conn);
static void conn_reset(conn_t *conn);
static size_t next_order(size_t size);

int init_conns()
{
    int ret = init_global_chead();
    return ret;
}
conn_t *get_free_conn()
{
	queue_t *que_sentinel;
	queue_t *que;

	if ( queue_empty( &global_chead.free_sentinel))
		return NULL;
	que_sentinel = &(global_chead.free_sentinel);
	que = que_sentinel->next;
	queue_remove(que);

	que_sentinel = &(global_chead.used_sentinel);
	queue_insert_head (que_sentinel, que);
     /*update the statics of the free and used connections*/
	global_chead.nr_free--;
	global_chead.nr_used++;

	conn_t *cn = queue_data(que, conn_t, queue);
    conn_init(cn);
	return cn;
}
void stop_conn(conn_t *conn)
{
    assert(conn);
    queue_t *que_sentinel;
	queue_t *que;

	que = &(conn->queue);
	queue_remove(que);

	que_sentinel = &(global_chead.free_sentinel);
	queue_insert_head(que_sentinel, que);
    
	global_chead.nr_free++;
	global_chead.nr_used--;
    conn_reset(conn);
}
void destroy_conns()
{
    assert(global_chead.conns);
    
    queue_t *que_sentinel, *que;
    que_sentinel = &global_chead.used_sentinel;
    while (!queue_empty(que_sentinel))
    {
        que = que_sentinel->next;
        queue_remove(que);
        conn_t *cn = queue_data(que, conn_t, queue);
        conn_reset(cn);
    }
    
    free(global_chead.conns);
	global_chead.conns = NULL;
	global_chead.nr_used = 0;
	global_chead.nr_free = 0;
}
void conn_append(conn_t *cn, char *data, size_t data_len)
{
    size_t dsize = data_len + cn->dsize;
    if (dsize >= cn->size)
    {
        size_t size = next_order(dsize);
        if(cn->buf)
        {
            cn->buf = Realloc(cn->buf, size);
        }
        else
        {
            if (size < INIT_SIZE)
            {
                size = INIT_SIZE;
            }
            cn->buf = Malloc(size);
        }
        cn->size = size;
    }
    memcpy(cn->buf + cn->dsize, data, data_len);
    cn->dsize = dsize;
    cn->buf[cn->dsize] = '\0';  
}
static int conn_init(conn_t *cn)
{
    assert(cn);
    cn->site = NULL;
    cn->url = NULL;
    cn->buf = NULL;
    cn->size = 0;
    cn->dsize = 0;
    cn->resp = http_resp_new();
    return 0;
}
static void conn_reset(conn_t *cn)
{
    Free(cn->buf);
    /*free the url_t object, do not free site_t object*/
    if (cn->url)
        url_destroy(cn->url);
    if(cn->resp)
        http_resp_destroy(cn->resp);
}
static int init_global_chead()
{
    queue_t *que_sentinel;
	queue_t *que;
	conn_t *conns_buf = calloc(1, NR_CONNS * sizeof(conn_t));
    if (!conns_buf)
    {
        fprintf(stderr, "calloc error\n");
        return -1;
    }
	global_chead.conns = conns_buf;
	global_chead.nr_free = NR_CONNS;
	global_chead.nr_used = 0;

	queue_init( &(global_chead.used_sentinel));
	queue_init( &(global_chead.free_sentinel));
	
	que_sentinel = &(global_chead.free_sentinel);
    int i;
	for (i = 0; i < NR_CONNS; i += 1)
	{
		/*insert the connection into the free list*/
		que = &(conns_buf->queue);
		queue_insert_tail(que_sentinel, que);
		
		conns_buf++;
	}
	
	return 0;
}

static size_t next_order(size_t size)
{
    size_t ret = 1;
    while(ret <= size)
    {
        ret = ret * 2;
    }
    return ret;
}

/* for debug */
#if 0
void print_stat()
{
    int count = 0;
    queue_t *que_sentinel = &global_chead.free_sentinel;
    queue_t *que = que_sentinel->next;
    while(que != que_sentinel)
    {
        count++;
        que = que->next;
    }
    printf("free: %d: %d\n", count, global_chead.nr_free);
    assert(count == global_chead.nr_free);
    
    count = 0;
    que_sentinel = &global_chead.used_sentinel;
    que = que_sentinel->next;
    while(que != que_sentinel)
    {
        count++;
        que = que->next;
    }
    printf("used: %d: %d\n\n", count, global_chead.nr_used);
    assert(count == global_chead.nr_used);
}

int main(int argc, char *argv[])
{
    conf_init();
    if (init_conns() < 0)
        return -1;
    print_stat();
    int i = 0;
    conn_t *conns[NR_CONNS];
    while(i < NR_CONNS)
    {
        conns[i] = get_free_conn();
        assert(conns[i]);
        i++;
    }
    print_stat();
    i = 0;
    while(i < NR_CONNS)
    {
        if (i % 2 == 0)
        {
            stop_conn(conns[i]);
            print_stat();
        }
        i++;
    }
    i = 0;
    while(i < NR_CONNS)
    {
        if (i % 2 == 0)
        {
            conns[i] = get_free_conn();
            print_stat();
        }
        i++;
    }
    return 0;
}
#endif
