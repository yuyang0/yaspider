#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#define Close(fd)  \
do{\
   if (fd != -1)  \
       close(fd);  \
   fd = -1;       \
}while(0)

#define Free(ptr)   \
do{ \
	if (ptr != NULL) \
		free(ptr);   \
	ptr = NULL;     \
}while(0)

void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
#endif