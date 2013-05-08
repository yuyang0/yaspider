#include "config.h"
#ifndef _STR_H_
#define _STR_H_

struct buf_str
{
    char *buf;
    size_t size;  //the size of buffer
    size_t used;  //the size used
};
typedef struct buf_str buf_str_t;
buf_str_t *buf_str_new(char *str);
void buf_str_append(buf_str_t *bstr, char *str);
void buf_str_destroy(buf_str_t *bstr);

#ifndef HAVE_STRDUP
char *strdup(const char *str);
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp(const char *, const char *);
#endif
#ifndef HAVE_STRCASESTR
char *strcasestr(char *str, char *fstr);
#endif
char *strnstr(char *haystack, char *needle, size_t n);
bool startswith(const char *, const char *);
bool endswith(const char *, const char *);
bool startscasewith(const char *, const char *);
bool endscasewith(const char *, const char *);


char *lstrip(char *str, char *d_chars);
char *rstrip(char *str, char *d_chars);
char *strip(char *str, char *d_chars);

char *strtolower(char *str);
char *strtoupper(char *str);

size_t strcountchr(char *str, char c);
size_t strcountstr(char *str, char *fstr);
#endif /* _STR_H_ */
