#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "wrapper.h"
void *Malloc(size_t size)
{
	char *ptr = calloc(1, size);
	if (ptr == NULL)
	{
		perror("malloc error:");
		exit(1);
	}
	return ptr;
}

void *Realloc(void *ptr, size_t size)
{
	assert(ptr);
	char *ret = realloc(ptr, size);
	if (!ret)
	{
		perror("realloc error");
		exit(1);
	}
	return ret;
}
