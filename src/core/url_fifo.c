#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "db.h" /*for the berkeley db*/
#include "fifo.h"
#include "url_fifo.h"

#define MAX_LINE 1024
#define FIXED_FIFO_SIZE 1024
typedef struct _url_fifo
{
    fifo_t *prior_fifo;       /*prior memory cached fifo */
    
	DB *normal_dbp;
    u_64 normal_hidx; /*the normal url fifo's head index*/ 
	u_64 normal_tidx;  /*the normal url fifo's end index*/
}url_fifo_t;

/* static function declaration */
static int open_db (DB **dbp, char *db_file, DBTYPE type);
static u_64 get_idx (DB *dbp, char *key_str);
static void store_idx (DB *dbp, char *key_str, u_64 idx);
static int get_val(DB *dbp, void  *k, size_t k_len, void *v, size_t v_len);
static int store_val(DB *dbp, void *k, size_t k_len, void *v, size_t v_len);

static url_fifo_t *url_fifo_new (char *normal_dbfile);
static void url_fifo_put_record (url_fifo_t *f, int priority, char *url_str);
static void *url_fifo_get_record (url_fifo_t * f, int priority);
static void url_fifo_destroy (url_fifo_t *url_dbp);

static void *_valDup(void *str)
{
    return strdup(str);
}
/*the global pointer to the url_fifo_t instance*/
static url_fifo_t *ufifo;

static url_fifo_t *url_fifo_new (char *normal_dbfile)
{
    url_fifo_t *uf = (url_fifo_t *)malloc (sizeof(*uf));
    if (uf == NULL)
        return NULL;
    if (!normal_dbfile)
    {
        normal_dbfile = "normal.db";
    }
    fifoType t = {&_valDup, &free};
    uf->prior_fifo = fifo_new(&t);

    open_db (&uf->normal_dbp, normal_dbfile, DB_BTREE);
    char head_str[] = "head_index";
    char tail_str[] = "tail_index";

    uf->normal_hidx = get_idx(uf->normal_dbp, head_str);
    uf->normal_tidx = get_idx(uf->normal_dbp, tail_str);
    return uf;
}

static void url_fifo_destroy (url_fifo_t *uf)
{
    if (uf == NULL)
        return;
    DB *dbp;
    char head_str[] = "head_index";
    char tail_str[] = "tail_index";
	
    u_64 head_idx;
    u_64 tail_idx;
    if (uf->normal_dbp)
    {
        dbp = uf->normal_dbp;
        head_idx = uf->normal_hidx;
        tail_idx = uf->normal_tidx;

        store_idx(dbp, head_str, head_idx);
        store_idx(dbp, tail_str, tail_idx);
        dbp->close (dbp, 0);
    }
    /* destroy priority fifo */
    fifo_destroy(uf->prior_fifo);
    free (uf);
}

/*if the prior is greater than 0,the record will put to prior url database*/
static void url_fifo_put_record (url_fifo_t *uf, int prior, char *url_str)
{
    DB *dbp;
    u_64 *tail_idxp;
    u_64 *head_idxp;
    if (prior > 0)
    {
    	fifo_put(uf->prior_fifo, url_str);
       	return;
    }
    else
    {
        tail_idxp = &uf->normal_tidx;
        head_idxp = &uf->normal_hidx;
        dbp = uf->normal_dbp;
	}
    if (*tail_idxp + 1  == *head_idxp)
    {
        fprintf(stderr, "too many url records in the database(0xffffffffffffffff)");
        return;
    }
    u_64 tail_idx = *tail_idxp;
    store_val(dbp, &tail_idx, sizeof(tail_idx), url_str, strlen(url_str) + 1);
    (*tail_idxp)++;
}
/*get and delete a record in db specified by prior*/
static void *url_fifo_get_record (url_fifo_t *uf, int priority)
{
    DB *dbp;
    u_64 *head_idxp;
    u_64 *tail_idxp;
    if (priority > 0)
    {
        return fifo_get(uf->prior_fifo);
    }
    else
    {
        tail_idxp = &uf->normal_tidx;
        head_idxp = &uf->normal_hidx;
        dbp = uf->normal_dbp;
    }
    /*the url fifo is empty*/
    if (*head_idxp == *tail_idxp)
        return NULL;

    u_64 head_idx = *head_idxp;
    
    char buf[MAX_LINE];

    int ret = get_val(dbp, &head_idx, sizeof(head_idx), buf, MAX_LINE);
    if (ret != 0)
    {
        dbp->err(dbp, ret, "DB->get:");
        return NULL;
    }

    (*head_idxp)++;
    return strdup(buf);
}

char *get_url_str(int priority)
{
    char *str = url_fifo_get_record (ufifo, priority);
    return str;
}

void put_url_str(char *url_str, int prior)
{
    url_fifo_put_record(ufifo, prior, url_str);
}
/* the number of prior urls*/
size_t prior_url_num()
{
	return fifo_num(ufifo->prior_fifo);
}
/* the number of normal urls */
size_t normal_url_num()
{
	if (ufifo->normal_tidx >= ufifo->normal_hidx)
	{
		return (ufifo->normal_tidx - ufifo->normal_hidx);
	}
	else
	{
		return 0xffffffffffffffff + (ufifo->normal_tidx - ufifo->normal_hidx);
	}
}
/*a wrapper function called by init all*/
int init_url_fifo(char *prior_file, char *normal_file)
{
    ufifo = url_fifo_new(normal_file);
    if (!ufifo)
    {
        return -1;
    }
    return 0;
}
void destroy_url_fifo()
{
    url_fifo_destroy(ufifo);
}
/*
 *@db_file : the database filename
 *@type    : must be set to one of DB_BTREE, DB_HASH, DB_HEAP, DB_QUEUE, DB_RECNO, or DB_UNKNOWN
 */
static int open_db (DB **dbpp, char *db_file, DBTYPE type)
{
    DB *dbp;
    u_int32_t flags;
    int ret;
    ret = db_create (&dbp, NULL, 0);
    if (ret != 0)
    {
        fprintf(stderr, "%s: %s\n", "program_name",
                db_strerror(ret));
        return(ret);
    }
    *dbpp = dbp;
    flags = DB_CREATE;
    ret = dbp->open (dbp, NULL, db_file, NULL, type, flags, 0);
    if (ret != 0)
    {
        dbp->err(dbp, ret, "Database '%s' open failed.", db_file);
        return(ret);
    }
    return 0;
}

//get the value of the key_str 
static u_64 get_idx(DB *dbp, char *key_str)
{
    u_64 idx = 0;
    if (get_val(dbp, key_str, strlen(key_str) + 1, &idx, sizeof(idx)) < 0)
    {
        idx = 0;
    }
    return idx;
}
static void store_idx(DB *dbp, char *key_str, u_64 idx)
{
    store_val(dbp, key_str, strlen(key_str), &idx, sizeof(idx));
}
/* get a value from berkeley DB */
static int get_val(DB *dbp, void *k, size_t k_len, void *v, size_t v_len)
{
    DBT key, value;
    memset(&key, 0, sizeof(DBT));
    memset(&value, 0, sizeof(DBT));
    key.data = k;
    key.size = k_len;
    value.data = v;
    value.ulen = v_len;
    value.flags = DB_DBT_USERMEM;
    int ret = dbp->get (dbp, NULL, &key, &value, 0);
    if (ret != 0)
    {
        return -1;
    }
    else
    {
        dbp->del(dbp, NULL, &key, 0);
        return 0;
    }
}
static int store_val(DB *dbp, void *k, size_t k_len, void *v, size_t v_len)
{
    DBT key, value;
    memset(&key, 0, sizeof(DBT));
    memset(&value, 0, sizeof(DBT));
    key.data = k;
    key.size = k_len;
    value.data = v;
    value.size = v_len;
    dbp->put(dbp, NULL, &key, &value, 0);
    return 0;
}
