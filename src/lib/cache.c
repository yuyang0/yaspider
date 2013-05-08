#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "wrapper.h"
#include "cache.h"
#define MAX_FREE_CACHES 1000
cache_head_t *cache_new (size_t size)
{
	cache_head_t *cache = Malloc (sizeof (cache_head_t));
	cache_init (cache, size);
	return cache;
}
void cache_init(cache_head_t *cache, size_t size)
{
	cache->size = size;
	cache->nr_free = 0;
	cache->nr_used = 0;
	queue_init(&cache->used_que);
	queue_init(&cache->free_que);
}

void *cache_malloc(cache_head_t *cache)
{
	cache_node_t *a_node;
	queue_t *queue;
	if (!queue_empty(&cache->free_que))
	{
		queue = queue_next(&cache->free_que);
		queue_remove(queue);
		a_node = queue_data(queue, cache_node_t, next);
		queue_insert_head(&cache->used_que, queue);

		cache->nr_used++;
		cache->nr_free--;
        /*set the memory to zero*/
		memset(a_node->data, 0, cache->size);
		return a_node->data;
	}
	else
	{
		size_t len = cache->size + sizeof(cache_node_t);
		a_node = Malloc(len);
		queue_insert_head(&cache->used_que, &a_node->next);
		a_node->data = (void *)(a_node+1);
		/*set the memory to zero*/
		memset(a_node->data, 0, cache->size);
		cache->nr_used++;

		return a_node->data;
	}
}

void cache_free(cache_head_t *cache, void *data)
{
	cache_node_t *a_node = data - sizeof(cache_node_t);
	queue_remove(&a_node->next);
	/*too many free cache objects? free it */
	if (cache->nr_free >= MAX_FREE_CACHES)
	{
		Free (a_node);
		cache->nr_used--;
		return;
	}
	queue_insert_head(&cache->free_que, &a_node->next);

	cache->nr_used--;
	cache->nr_free++;
}
#if (0)
int main()
{
	cache_head_t cache;
	init_cache(&cache, 8);
	long long *data = cache_malloc(&cache);
	*data = 111111111;
	cache_free(&cache, data);
	data = cache_malloc(&cache);
	cache_free(&cache, data);
	return 0;
}
#endif
