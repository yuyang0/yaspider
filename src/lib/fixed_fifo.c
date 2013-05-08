/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  fixed_fifo.c
 *        Created:  2013-05-02 16:57:25
 *       Compiler:  gcc
 *    Description:  A implement of fixed size fifo
 *
 *         Author:  Yu Yang
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
#include <stdbool.h>

#include "fixed_fifo.h"
static void *_valDup(fixed_fifo_t *f, void *val);
fixed_fifo_t *fixed_fifo_new(size_t size, fixedFifoType *t)
{
    fixed_fifo_t *f = calloc(1, sizeof(*f));
    if (!f)
    {
        return NULL;
    }
    f->type = malloc(sizeof(*t));
    if (!f->type)
    {
        free(f);
        return NULL;
    }
    memcpy(f->type, t, sizeof(*t));
    f->in = 0;
    f->out = 0;
    /* 需要空出一个元素来检查fifo是否已满 */
    f->tab = calloc(size + 1, sizeof(void *));
    if (!f->tab)
    {
        free(f->type);
        free(f);
        return NULL;
    }
    f->size = size + 1;
    return f;
}
int fixed_fifo_destroy(fixed_fifo_t *f)
{
    if (!f)
    {
        return -1;
    }
    void *val = fixed_fifo_get(f);
    for (; val; val = fixed_fifo_get(f))
    {
        freeVal(f, val);
    }
    free(f->tab);
    free(f);
    return 0;
}
void *fixed_fifo_get(fixed_fifo_t *f)
{
    if (f->in == f->out)
    {
        return NULL;
    }
    void *p = (f->tab)[f->out];
    f->out = (f->out + 1) % f->size;
    return p;
}
int fixed_fifo_put(fixed_fifo_t *f, void *entry)
{
    if (fixed_fifo_full(f))
    {
        return -1;
    }
    f->tab[f->in] = _valDup(f, entry);
    f->in = (f->in + 1) % f->size;
    return 0;
}
bool fixed_fifo_empty (fixed_fifo_t *f)
{
    if (f->in == f->out)
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool fixed_fifo_full(fixed_fifo_t *f)
{
    size_t next = (f->in + 1) % f->size;
    if (next == f->out)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static void *_valDup(fixed_fifo_t *f, void *val)
{
    if (f->type && f->type->valDup)
    {
        return f->type->valDup(val);
    }
    else
    {
        return val;
    }
}
