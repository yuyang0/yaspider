/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  fixed_fifo.h
 *        Created:  2013-05-02 16:57:34
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _FIXED_FIFO_H_
#define _FIXED_FIFO_H_ 1
typedef struct _fixedFifoType
{
    void *(*valDup)(void *val);
    void (*valDestructor)(void *val);
}fixedFifoType;

typedef struct _fixed_fifo
{
    fixedFifoType *type;
    void **tab;
    size_t in;
    size_t out;
    size_t size;
}fixed_fifo_t;
#define freeVal(f, _val_)                       \
    do{                                         \
        if(f->type && f->type->valDestructor)               \
            f->type->valDestructor(_val_);       \
    }while(0)

fixed_fifo_t *fixed_fifo_new(size_t size, fixedFifoType *t);
int fixed_fifo_destroy(fixed_fifo_t *f);
void *fixed_fifo_get (fixed_fifo_t *f);
int fixed_fifo_put (fixed_fifo_t *f, void *entry);
bool fixed_fifo_empty(fixed_fifo_t *f);
bool fixed_fifo_full(fixed_fifo_t *f);
#endif /* _FIXED_FIFO_H_ */

