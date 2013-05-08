/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  siteht.c
 *        Created:  2013-04-14 11:15:33
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"
#include "url.h"
#include "site.h"
#include "siteht.h"
static dict *site_dict;
static unsigned int _siteht_hash_function(const void *key)
{
    const char *host = key;
    return dictGenHashFunction(host, strlen(host));
}
static int _siteht_key_compare(void *private, const void *key1, const void *key2)
{
    DICT_NOTUSED(private);
    return strcmp(key1, key2) == 0;
}
static void *_siteht_key_dup(void *private, const void *key)
{
    DICT_NOTUSED(private);
    char *new_key = malloc(strlen(key) + 1);
    if (new_key)
        strcpy(new_key, key);
    return new_key;
}
static void _siteht_key_destructor(void *private, void *key)
{
    DICT_NOTUSED(private);
    if (key)
        free(key);
}
static void _siteht_val_destructor(void *private, void *val)
{
    DICT_NOTUSED(private);
    site_t *s = val;
    if (s)
        site_destroy(s);
}
static dictType _siteht_type = {
    _siteht_hash_function,
    _siteht_key_dup,
    NULL,
    _siteht_key_compare,
    _siteht_key_destructor,
    _siteht_val_destructor
};

void siteht_init()
{
    site_dict = dictCreate(&_siteht_type, NULL);
}

/* return NULL if host is not in the hash table*/
site_t *siteht_get_site(char *host)
{
    site_t *s = dictFetchValue(site_dict, host);
    return s;
}
/*
  return 0 if the host is already in the hash table
  otherwith return 1
*/
int siteht_set(char *host, site_t *s)
{
    return dictReplace(site_dict, host, s);
}

void siteht_release()
{
    dictRelease(site_dict);
}


/* debug******************/
#if 0
#include "str.h"
#define MAXLINE 1024
static char *get_host(char *url_str)
{
    static char host[MAXLINE];
    memset(host, 0, MAXLINE);
    char *host_start = url_str;
    char *host_end;
    if (startswith(url_str, "http://"))
        host_start += 7;
    else if (startswith(url_str, "https://"))
        host_start += 8;
    char *port = strchr (host_start, ':');
    char *file = strchr(host_start, '/');
    if (port)
    {
        host_end = port;
    }
    else if (file)
    {
        host_end = file;
    }
    else
    {
        host_end = url_str + strlen(url_str);
    }
    int host_len = host_end - host_start;
    memcpy(host, host_start, host_len);
    return host;
}

char *url[] = {
    "http://news.sina.com.cn/z/zhonggongshibada/",
    "http://login.sina.com.cn/",
    "http://mail.sina.com.cn/",
    "http://weibo.com/",
    "http://blog.sina.com.cn/u/1891379660",
    "http://you.video.sina.com.cn/m/1891379660",
    "http://bbs.sina.com.cn/",
    "http://login.sina.com.cn/",
    "http://login.sina.com.cn/member/security/password.php",
    "https://login.sina.com.cn/sso/logout.php",
    "http://weibo.com/",
    "http://login.sina.com.cn/",
    "http://help.sina.com.cn/index.php",
    "http://mail.sina.net/",
    "http://sina.cn/",
    "http://news.sina.com.cn/guide/",
    "http://news.sina.com.cn/",
    "http://mil.news.sina.com.cn/",
    "http://news.sina.com.cn/society/",
    NULL
};
void print_site(site_t *si)
{
    while (!site_empty(si))
    {
        url_t *u = site_get_url(si);
        printf("%s\n       %s   %s\n\n", u->full_url, u->host, u->file);

    }
}
int main(int argc, char const *argv[])
{
    siteht_init();
    char **ptr = url;
    while (*ptr)
    {
        //printf("%s\n", get_host(*ptr));
        char *host = get_host(*ptr);
        site_t *si = siteht_get_site(host);
        //printf("%p\n", si);
        if (!si)
        {
            si = site_new();
            siteht_set(host, si);
        }
        url_t *u = url_new_with_str(*ptr);
        //printf("%s\n       %s   %s\n\n", u->full_url, u->host, u->file);
        site_put_url(si, u);
        ptr++;
    }

    ptr = url;
    while(*ptr)
    {
        char *host = get_host(*ptr);
        site_t *si = siteht_get_site(host);
        printf("%p\n", si);
        print_site(si);
        ptr++;
    }
    return 0;
}
#endif