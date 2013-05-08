#ifndef _CACHE_H_
#define _CACHE_H_
#include "queue.h"
typedef struct cache_head_s cache_head_t;
typedef struct cache_node_s cache_node_t;

struct cache_head_s
{
	int size;
	int nr_free;
	int nr_used;
	
	queue_t free_que;
	queue_t used_que;
};
struct cache_node_s
{
	void *data;
	queue_t next;
};
cache_head_t *cache_new (size_t size);
void cache_init(cache_head_t *cache, size_t size);
void *cache_malloc(cache_head_t *cache);
void cache_free(cache_head_t *cache, void *data);
#endif