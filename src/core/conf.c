/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  conf.c
 *        Created:  2013-04-14 18:07:47
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "str.h"
#include "conf.h"

#define MAXLINE 1024
#define SEP ' '
conf_t settings;   //the global conf object

/* private functions */
static char *get_key_val(FILE *fp, char *key);
static char *skip_head_char(char *line, char c);

static bool is_comment_line(char *line);
static bool is_blank_line(char *line);

int get_int_val(FILE *fp, char *entry);
char *get_str_val(FILE *fp, char *entry);
bool get_bool_val(FILE *fp, char *entry);
void get_multi_str_val(FILE *fp, char *entry, char **ret, int len);

static void free_arr(char **arr);
/*********end****************/

int conf_init(char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        return -1;
    }
    settings.user_agent    = get_str_val(fp, "user_agent");
    settings.port          = get_int_val(fp, "server_port");
    settings.max_ram_urls  = get_int_val(fp, "max_ram_urls");
    settings.max_site_urls = get_int_val(fp, "max_site_urls");
    settings.timeout       = get_int_val(fp, "connection_timeout");
    settings.interval      = get_int_val(fp, "wait_duration");
    settings.max_conns     = get_int_val(fp, "max_conns");
    settings.max_dns_calls = get_int_val(fp, "max_dns_calls");
    settings.daemon        = get_bool_val(fp, "daemon");
    settings.save          = get_int_val(fp, "save");
    settings.save_path     = get_str_val(fp, "save_path");
    settings.have_server   = get_bool_val(fp, "have_server");

    get_multi_str_val(fp, "start_urls", settings.urls, MAX_START_URLS);
    get_multi_str_val(fp, "prior_exts", settings.prior_exts, MAX_EXTS);
    get_multi_str_val(fp, "forb_exts", settings.forb_exts, MAX_EXTS);
    get_multi_str_val(fp, "allowed_domain", settings.allowed_domain, MAX_DOMAIN);
    get_multi_str_val(fp, "prior_domain", settings.prior_domain, MAX_DOMAIN);
    return 0;
}
void conf_reset()
{
    if (settings.user_agent)
    {
        free(settings.user_agent);
    }
    if (settings.save_path)
    {
        free(settings.save_path);
    }
    free_arr(settings.urls);
    free_arr(settings.prior_exts);
    free_arr(settings.forb_exts);
    free_arr(settings.allowed_domain);
    free_arr(settings.prior_domain);
}
static void free_arr(char **arr)
{
    if (!arr)
    {
        return;
    }
    char **ptr = arr;
    for (; *ptr; ++ptr)
    {
        free(*ptr);
    }
}
int get_int_val(FILE *fp, char *entry)
{
    char *val = get_key_val(fp, entry);
    if (val)
    {
        return atoi(val);
    }
    else
    {
        return -1;
    }
}
char *get_str_val(FILE *fp, char *entry)
{
    char *val = get_key_val(fp, entry);
    if (val)
    {
        return strdup(val);
    }
    else
    {
        return NULL;
    }
}
bool get_bool_val(FILE *fp, char *entry)
{
    char *val = get_key_val(fp, entry);
    if (val)
    {
        if (!strcasecmp(val, "on"))
        {
            return true;
        }
        else if (!strcasecmp(val, "off"))
        {
            return false;
        }
        else
        {
            fprintf(stderr, "only 'on' and 'off' is valid for the key %s\n", entry);
            return false;
        }
    }
    else
    {
        fprintf(stderr, "not found %s\n", entry);
        return false;
    }
}
/* the code of this function is dirty*/
void get_multi_str_val (FILE *fp, char *entry, char **ret, int len)
{
    assert(fp && entry && ret);
    fseek(fp, 0, SEEK_SET);
    char line[MAXLINE];
    bool in_val = false;
    
    while (fgets(line, MAXLINE, fp) != NULL)
    {
        if (is_comment_line(line) || is_blank_line(line))
        {
            continue;
        }
        char *end = line + strlen(line);
        if (*(end - 1) == '\n')
        {
            *(end - 1) = '\0';
        }
        char *start = strip(line, " ");
        if (in_val)
        {
            if (strchr (start, '}'))
            {
                in_val = false;
                return;
            }
            else
            {
                char *val_start = strtok(start, " ");
                for (; ; )
                {
                    /* the end element of the array should be NULL*/
                    if (len > 1)
                    {
                        *ret = strdup(val_start);
                        ret++;
                        len--;
                        val_start = strtok(NULL, " ");
                        if (!val_start)
                        {
                            break;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "entry %s has too many values\n", entry);
                        return;
                    }
                }
            }
        }
        char *middle = strchr(start, SEP);
        if (!middle)
        {
            continue;
        }
        *middle = '\0';
        middle++;
        char *name = strip(start, " ");
        if (strcasecmp(name, entry))
        {
            continue;
        }
        else
        {
            char *value = strip(middle, " ");
            if (!strchr (value, '{'))
            {
                fprintf(stderr, "error: not found '{' for entry: %s\n", entry);
                return;
            }
            else
            {
                in_val = true;
            }
        }
    }
    return;
}
static char *get_key_val(FILE *fp, char *key)
{
    assert(fp && key);
    fseek(fp, 0, SEEK_SET);
    static char line[MAXLINE];
    while (fgets(line, MAXLINE, fp) != NULL)
    {
        if (is_comment_line(line) || is_blank_line(line))
        {
            continue;
        }
        char *start = skip_head_char(line, ' ');
        char *end = line + strlen(line);
        if (*(end - 1) == '\n')
        {
            *(end - 1) = '\0';
        }
        char *middle = strchr(start, SEP);
        if (!middle)
        {
            continue;
        }
        *middle = '\0';
        middle++;
        char *name = strip(start, " ");
        if (strcasecmp(name, key))
        {
            continue;
        }
        else
        {
            char *value = strip(middle, " ");
            return value;
        }
    }
    return NULL;
}
static char *skip_head_char(char *line, char c)
{
    while(*line == c)
    {
        line++;
    }
    return line;
}
static bool is_comment_line(char *line)
{
    while(*line == ' ')
        line++;
    if(*line == '#')
        return true;
    else
        return false;
}
static bool is_blank_line(char *line)
{
    while(*line == ' ')
        line++;
    return (*line == '\n')? true : false;
}


/* debug  */
#if 0
int main(int argc, char *argv[])
{
    if (conf_init("../conf/settings.conf") < 0)
    {
        fprintf(stderr, "conf init error\n");
        return -1;
    }
    
    return 0;
}
#endif
