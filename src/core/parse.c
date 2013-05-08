/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  parse.c
 *        Created:  2013-04-20 12:41:45
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include "core.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define URL_LENGTH 4096
/****private function********************************/
static bool check_forb_exts(char *url);
static bool check_bf (char *url);
static bool check_allowed_domain(char *url);

static bool check_prior_exts(char *url);
static bool check_prior_domain(char *url);

static int parse_robots_raw(char *buf, char **forb_paths, char *spider_name);
/************end************************************/
/**test ok*********/
void fetch_all_urls(conn_t *cn)
{
    char *buf = cn->resp->body;
    if (!buf)
    {
        log_error("the body is empty: %s", cn->url->full_url);
        return;
    }
    char *tag = strcasestr(buf, "<a ");
    while(tag)
    {
        char *href = strcasestr(tag, "href");
        char *tag_end = strstr(tag, ">");
        if (!tag_end || !href)
        {
            break;
        }
        char *val = href + strlen("href");
        if (val > tag_end)
        {
            goto next_loop;
        }
        while(*val == ' ' ||
              *val == '=' ||
              *val == '"')
        {
            val++;
        }
        char *val_end = strchr(val, ' ');
        if (!val_end || val_end > tag_end)
        {
            val_end = tag_end;
        }
        while(*(val_end - 1) == ' '||
              *(val_end - 1) == '"')
        {
            val_end--;
        }
        size_t len = val_end - val;
        /* the url is too long, skip it*/
        if (len >= URL_LENGTH)
        {
            goto next_loop;
        }
        char url[URL_LENGTH] = {0};
        memset(url, 0, MAXLINE);
        if (!startswith(val, "http"))
        {
            if (!startswith(val, "/"))
            {
                goto next_loop;
            }
            else
            {
                /* a relative address*/
                strcpy(url, cn->url->protocol);
                strcpy(url + strlen(url), "://");
                strcpy(url + strlen(url), cn->url->host);
                if (strlen(url) + len >= URL_LENGTH)
                {
                    goto next_loop;
                }
            }
        }
        memcpy(url + strlen(url), val, len);
        if (!verify_url(url))
        {
            goto next_loop;
        }
        int prior = is_prior(url)? 1 : 0;
        put_url_str(url, prior);
    next_loop:
        tag = strcasestr(tag_end, "<a ");
    }
}
int parse_robots(conn_t *cn)
{
    parse_robots_raw(cn->resp->body, cn->site->forb_paths, settings.user_agent);
}
/* parse robots.txt */
static int parse_robots_raw(char *buf, char **forb_paths, char *spider_name)
{
    if (!buf || !forb_paths || !spider_name)
    {
        log_error("parse_robots_raw: the body(robots.txt) is empty");
        return -1;
    }
	const int nr_forb_paths = MAX_FORB_PATHS - 1;
	int i = 0;

	char *line_start = strtok (buf, "\r\n");
	bool contain_me = false;  /*the useragent contain our spider*/
	char *line_end;
	while(line_start)
	{
		line_start = lstrip(line_start, " ");
		/*skip the comments line and space line*/
		if (*line_start == '#' || *line_start == '\0')
			goto next_loop;

		char *line_middle = strchr(line_start, ':');
		if (line_middle == NULL)
			break;
		*line_middle = '\0';
		char *entry_name = strip(line_start, " ");
		char *entry_value = strip(line_middle + 1, " ");
		if (!strcasecmp(entry_name, "user-agent") ||
            !strcasecmp(entry_name, "useragent"))
		{
			if (!strcmp(entry_value, "*") || strcasestr(entry_value, spider_name) != NULL)
				contain_me = true;
			else
				contain_me = false;
		}
		else if (!strcasecmp(entry_name, "disallow"))
		{
			if (contain_me == false)
				goto next_loop;
			if (i >= nr_forb_paths)
				break;
			forb_paths[i++] = strdup(entry_value);
		}
    next_loop:
		line_start = strtok(NULL, "\r\n");
	}
}
/* we don't check robots now,because the site may not
   have gotten robots.txt
 */
bool verify_url(char *url)
{
    if (check_forb_exts(url) &&
        check_bf(url)       &&
        check_allowed_domain(url))
    {
        return true;
    }
    else
    {
        return false;
    }
}
/* if the url is a priority url ,return true
   else return false
 */
bool is_prior(char *url)
{
    if (check_prior_exts(url) ||
        check_prior_domain(url))
    {
        return true;
    }
    else
    {
        return false;
    }
}
/* return true if urls's extension is not in forbidden extensions
   otherwith return false
*/
static bool check_forb_exts(char *url)
{
    char **p = settings.forb_exts;
    while(*p)
    {
        if (endscasewith(url, *p))
        {
            return false;
        }
        p++;
    }
    return true;
}
/* return true if url is not already in bloom filter
   otherwith return false
 */
static bool check_bf(char *url)
{
    if (url_bf_test(url))
    {
        return false;
    }
    else
    {
        url_bf_add(url);
        return true;
    }
}
/* return true if url's domian is in allowed domains
   otherwith return false
 */
static bool check_allowed_domain(char *url)
{
    if (!settings.allowed_domain[0])
    {
        return true;
    }
    else
    {
        char *host = get_host(url);
        char **p = settings.allowed_domain;
        while(*p)
        {
            if (endscasewith(host, *p))
            {
                return true;
            }
            p++;
        }
        return false;
    }
}
/* return true if url's extension is in priority extension array
   otherwith return false
 */
static bool check_prior_exts(char *url)
{
    if (!settings.prior_exts)
    {
        return false;
    }
    char **ptr = settings.prior_exts;
    while(*ptr)
    {
        if (endscasewith(url, *ptr))
        {
            return true;
        }
        ptr++;
    }
    return false;
}
/* return true if url's domian is in priority domain array
   otherwith return false
 */
static bool check_prior_domain(char *url)
{
    if (!settings.prior_domain)
    {
        return false;
    }
    char *host = get_host(url);
    char **ptr = settings.allowed_domain;
    while(*ptr)
    {
        if (endscasewith(host, *ptr))
        {
            return true;
        }
        ptr++;
    }
    return false;
}

char *get_base_url(char *buf)
{
    if (!buf)
    {
        log_error("get_base_url: the buf is NULL");
        return NULL;
    }
	char *header_start = strcasestr(buf, "<head>");
	char *header_end   = strcasestr(buf, "</head>");
	char *base_ele_start = strcasestr(header_start, "<base ");
	char *base_ele_end  = strchr(base_ele_start, '>');
	if (base_ele_start == NULL || base_ele_end == NULL || base_ele_start >= header_end || base_ele_end >= header_end)
		return NULL;
	char *url_start = strcasestr(base_ele_start, "href");
	url_start += 4;
	while(*url_start == '=' || *url_start == '"' || *url_start == ' ')
		url_start++;
	char *url_end = strchr(url_start, '"');
	int url_len = url_end - url_start;
	char *base_url = malloc (url_len + 1);
    if (!base_url)
    {
        log_error("malloc error");
        return NULL;
    }
	memcpy(base_url, url_start, url_len);
	base_url[url_len] = '\0';
	return base_url;
}

// static char *next_line(char *buf)
// {
//     static char *start = NULL;
//     static char *end = NULL;
//     if (!buf && !start)
//     {
//         fprintf(stderr, "next_line: incorrect argument(buf is NULL)\n");
//         return NULL;
//     }
//     if (buf)
//     {
//         start = buf;
//         end = strchr(start, '\n');
//         if (end)
//         {
//             *end = '\0';
//             end++;
//         }
//         else
//         {
//             end = buf + strlen(buf);
//         }
//         return start;
//     }
//     start = end + 1;
//     end = strchr(start, '\n');
//     if (!end)
//     {
//         return NULL;
//     }
//     else
//     {
//         pos = p + 1;
//         return pos;
//     }

// }