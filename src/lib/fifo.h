#ifndef _FIFO_H_
#define _FIFO_H_
#include <stdio.h>
#include <stdbool.h>
typedef struct fifo_s fifo_t;
typedef struct fifoType
{
    void *(*valDup)(void *val);
    void (*valDestructor)(void *val);
}fifoType;
struct fifo_s
{
    fifoType *type;
	size_t in;
	size_t out;
	size_t size;
	void **tab;
};
#define freeVal(f, _val_)                       \
    do{                                         \
        if(f->type && f->type->valDestructor)               \
            f->type->valDestructor(_val_);       \
    }while(0)

fifo_t *fifo_new(fifoType *t);
int fifo_init(fifo_t *f, fifoType *t);
bool fifo_empty(fifo_t *fifo);
void *fifo_get(fifo_t *fifo);
int fifo_put(fifo_t *fifo, void *p);
void fifo_reput(fifo_t *fifo, void *p);
size_t fifo_num(fifo_t *f);

void fifo_reset(fifo_t *fifo);
void fifo_destroy(fifo_t *fifo);
#endif
