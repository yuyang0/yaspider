#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "fifo.h"

#define FIFO_INIT_SIZE 8

static void *_valDup(fifo_t *f, void *val);

fifo_t *fifo_new(fifoType *t)
{
    fifo_t *f = malloc(sizeof(fifo_t));
    if (f)
    {
        if (fifo_init(f, t) < 0)
        {
            free(f);
            return NULL;
        }
    }
    return f;
}
int fifo_init(fifo_t *f, fifoType *t)
{
    assert(f);
    /* copy the type structure */
    if (t)
    {
        f->type = malloc (sizeof (*t));
        if (!f->type)
        {
            return -1;
        }
        memcpy (f->type, t, sizeof(*t));
    }
    else
    {
        f->type = NULL;
    }
    

    f->tab = calloc(1, sizeof(void*) * FIFO_INIT_SIZE);
    if (!f->tab)
    {
        free(f->type);
        return -1;
    }
    f->size = FIFO_INIT_SIZE;
    f->in = 0;
    f->out = 0;
    return 0;
}
/*return true if fifo is empty otherwith return false*/
bool fifo_empty(fifo_t *fifo)
{
    if (fifo->in == fifo->out)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void *fifo_get (fifo_t *fifo) 
{
    if (fifo->in != fifo->out)
    {
        void *val = (fifo->tab)[fifo->out];
        fifo->out = (fifo->out + 1) % fifo->size;
        return val;
    }
    else
    {
        return NULL;
    }
}

int fifo_put (fifo_t *f, void *obj)
{
    (f->tab)[f->in] = _valDup(f, obj);
    f->in = (f->in + 1) % f->size;
    /* the fifo is full */
    if (f->in == f->out)
    {
        void **tmp;
        tmp = calloc(1, sizeof(void *) * 2 * f->size);
        if (!tmp)
        {
            return -1;
        }
        size_t i;
        for (i = f->out; i < f->size; i++)
            tmp[i] = (f->tab)[i];
        for (i=0; i < f->in; i++)
            tmp[i + f->size] = (f->tab)[i];
        f->in += f->size;
        f->size *= 2;
        free(f->tab);
        f->tab = tmp;
    }
    return 0;
}

void fifo_reput (fifo_t *f, void *obj)
{
    (f->tab)[f->in] = _valDup(f, obj);
    f->in = (f->in + 1) % f->size;
}
/* the number of elements in the fifo */
size_t fifo_num(fifo_t *f)
{
    assert(f);
    if (f->in >= f->out)
    {
        return (f->in - f->out);
    }
    else
    {
        return (f->size + f->in - f->out);
    }
}
void fifo_reset(fifo_t *f)
{
    assert(f);
    void *v;
    while((v = fifo_get(f)))
        freeVal(f, v);
    free(f->tab);
}
void fifo_destroy(fifo_t *f)
{
    assert(f);
    fifo_reset(f);
    free(f);
}

static void *_valDup(fifo_t *f, void *val)
{
    void *v = val;
    if (f->type && f->type->valDup)
        v = f->type->valDup(val);
    return v;
}
