#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "wrapper.h"
#include "str.h"
#include "fifo.h"
#include "url.h"
#include "site.h"

static bool pattern_match(char *pattern, char *path);
static void _val_free(void *u)
{
    url_destroy(u);
}

site_t *site_new()
{
    site_t *si = Malloc(sizeof(site_t));
    if (!si)
        return NULL;
    if (site_init(si) < 0)
    {
        Free(si);
        return NULL;
    }
    return si;
}
int site_init(site_t *si)
{
    assert(si);
    /* the site's fifo type*/
    fifoType ty = {
    NULL,
    &_val_free
    };

    si->url_fifo = fifo_new(&ty);
    if (!si->url_fifo)
        return -1;
    si->dns_state = WAIT_DNS;
    si->ram_urls = 0;
    si->in_wait_fifo = false;
    si->in_ready_fifo = false;
    si->host = NULL;
    si->next_call = 0;
    si->robots_gotten = false;
    return 0;
}
void site_destroy(site_t *si)
{
    assert(si);
    
    fifo_destroy(si->url_fifo);
    Free(si->host);
    Free(si->cname);
    Free(si);
}

bool site_empty(site_t *si)
{
    return fifo_empty(si->url_fifo);
}
/*url_t objects*/
url_t *site_get_url(site_t *si)
{
    assert(si);
    if (site_empty(si))
        return NULL;
    return (url_t *)fifo_get(si->url_fifo);
}
/*the url should be url_t */
void site_put_url(site_t *si, url_t *u)
{
    /* the first time add a url to site's fifo*/
    if (!si->host)
        si->host = strdup(u->host);
    fifo_put(si->url_fifo, u);
    si->ram_urls++;
}
/*if path is not forbidden by robots.txt return true, otherwith return false*/
bool site_test_robots(site_t *si, char *path)
{
	int i;
	for (i = 0; i < MAX_FORB_PATHS && si->forb_paths[i] != NULL; ++i)
	{
		char *entry = si->forb_paths[i];
		if (pattern_match(entry, path))
			return false;
	}
	return true;
}
/*return true if the file is matched by the pattern otherwith return false*/
static bool pattern_match(char *pattern, char *path)
{
	int i = 0;
	char *path_pos = path;
	while(pattern[i] != '\0')
	{
		if (pattern[i] == '*')
		{
			++i;
			path_pos = strchr(path_pos, pattern[i]);
			if (path_pos == NULL)
				return false;
		}
		else
		{
			if (pattern[i] != *path_pos)
				return false;
            i++;
            path_pos++;
		}
	}
	return true;
}
