/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  str.c
 *        Created:  2013-03-22 22:14:51
 *       Compiler:  gcc
 *    Description:  some string utils
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#define Free(p)   \
    do{\
    if (p != NULL)\
        free(p);  \
    p = NULL;\
    }while(0)
static void *Malloc(size_t size);
buf_str_t *buf_str_new(char *str)
{
    size_t str_len = strlen(str);
    buf_str_t *bstr = Malloc(sizeof(buf_str_t));
    bstr->buf = Malloc(str_len * 2);
    strcpy(bstr->buf, str);
    bstr->size = str_len * 2;
    bstr->used = str_len;
    return bstr;
}
void buf_str_append(buf_str_t *bstr, char *str)
{
    size_t str_len = strlen(str);
    size_t free_size = bstr->size - bstr->used - 1;
    if (free_size < str_len)
    {
        /*realloc the memory*/
        char *p = Malloc((bstr->used + str_len) * 2);
        strcpy(p, bstr->buf);
        free(bstr->buf);
        bstr->buf = p;
        
        bstr->size = (bstr->used + str_len) * 2;
    }
    strcpy((bstr->buf + bstr->used), str);
    bstr->used += strlen(str);
 }
void buf_str_destroy (buf_str_t *bstr)
{
    Free(bstr->buf);
    Free(bstr);
}

#ifndef HAVE_STRDUP
char *strdup(const char *str)
{
    char *buf = Malloc(strlen(str) + 1);
    strcpy(buf, str);
    return buf;
}
#endif

#ifndef HAVE_STRCASECMP
/*compare two string ignore case*/
int strcasecmp(const char *str1, const char *str2)
{
    while(*str1 && *str2 && ((*str1 | 32) == (*str2 | 32)))
    {
        str1++;
        str2++;
    }
    return (*str1 - *str2);
}
#endif

#ifndef HAVE_STRCASESTR
char *strcasestr(const char *str, const char *fstr)
{
    do{
        if(startscasewith(str, fstr))
            return str;
        else
            str++;
    } while (*str);
    return NULL;
}
#endif

char *strnstr(char *haystack, char *needle, size_t n)
{
    int i;
    char *p = haystack;
    for (i = 0; i < n; ++i)
    {
        if (startswith(p, needle))
        {
            return p;
        }
        else
        {
            p++;
        }
    }
    return NULL;
}
//test ok
bool startswith(const char *str, const char *prefix)
{
    if (strncmp(str, prefix, strlen(prefix)))
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool endswith(const char *str, const char *suffix)
{
    if(strlen(str) < strlen(suffix))
        return false;
    const char *p = str + (strlen(str) - strlen(suffix));
    return startswith(p, suffix);
}
bool startscasewith(const char *str, const char *prefix)
{
    while(*str && *prefix && ((*str | 32) == (*prefix | 32)))
    {
        str++;
        prefix++;
    }
    if (*prefix == '\0')
        return true;
    else
        return false;
}
bool endscasewith(const char *str, const char *suffix)
{
    if(strlen(str) < strlen(suffix))
        return false;
    const char *p = str + (strlen(str) - strlen(suffix));
    return startscasewith(p, suffix);
}
/* return true if the string contain the character
   otherwith return false
 */
static bool contain_char(char *s, char c)
{
    for (; *s; ++s)
    {
        if (*s == c)
        {
            return true;
        }
    }
    return false;
}
/*strip the characters contained in d_chars
  at the beginning of the string
*/
char *lstrip(char *str, char *d_chars)
{
    for (; *str; ++str)
    {
        char c = *str;
        if (!contain_char(d_chars, c))
        {
            break;
        }
    }
    return str;
}
char *rstrip(char *str, char *d_chars)
{
    char *end = str + strlen(str) - 1;
    for (; end >= str; --end)
    {
        char c = *end;
        if (!contain_char(d_chars, c))
        {
            break;
        }
    }
    *(++end) = '\0';
    return str;
}
char *strip(char *str, char *d_chars)
{
    char *start = lstrip(str, d_chars);
    return rstrip(start, d_chars);
}
//test ok
char *strtolower(char *str)
{
    char *ret = str;
    while(*str)
    {
        if(*str >= 65 && *str <= 90)
            *str |= 32;
        str++;
    }
    return ret;
}
//test ok
char *strtoupper(char *str)
{
    char *ret = str;
    while(*str)
    {
        if (*str >= 97 && *str <= 122)
            *str &= (~32);
        str++;
    }
    return ret;
}
size_t strcountchr(char *str, char c)
{
    size_t count = 0;
    while (*str)
    {
        if (*str == c)
            count++;
        str++;
    }
    return count;
}
size_t strcountstr(char *str, char *fstr)
{
    size_t count = 0;
    while (*str)
    {
        if (startswith(str, fstr))
        {
            count++;
            str += strlen(fstr);
            continue;
        }
        str++;
    }
    return count;
}

static void *Malloc(size_t size)
{
    void *buf = calloc(1, size);
    if (buf == NULL)
    {
        perror("Malloc error");
        exit(1);
    }
    return buf;
}

#if 0
char *upper_str[] = {
	"WWW.BAIDU.COM",
	"WWW.SINA.COM",
	"WWW.163.COM",
	"WWW.SOHU.COM",
	NULL
};
char *lower_str[] = {
	"www.baidu.com",
	"www.sina.com",
	"www.163.com",
	"www.sohu.com",
	NULL
};
int main(int argc, char *argv[])
{
    /*test strtoupper*/
    char buf_low[] = "hello world A Z a z $ # & *";
    char buf_upp[] = "HELLO WORLD hello  A Z a z $ % & * )";
    printf ("%s\n",strtoupper(buf_low));
    printf ("%s\n",strtolower(buf_upp));

    char buf[] = "    hello world   ";
    char ss[] = "  = & aa = &bb = & =  =";
    printf ("%s end\n",strip(ss, " =&"));
    printf ("%s end\n", strip(buf, " "));
    printf ("startswith:%d\n\n",startswith("hello", "hel1"));
    printf ("endswith:%d\n",endswith("hello", "ello"));
    printf ("startscasewith:%d\n",startscasewith("HeLlo", "Ell"));
    printf ("strcasestr:%s\n",strcasestr("wHO Are YOU", " aRe you"));
    return 0;
}
#endif
