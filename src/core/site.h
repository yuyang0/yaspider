#ifndef _SITE_H_
#define _SITE_H_
#include <stdint.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "fifo.h"
#define MAX_ADDRS 10
#define MAX_FORB_PATHS 128
/* different state of dnsQuery*/
enum dns_ste{
    WAIT_DNS,
    DONE_DNS,
    ERROR_DNS,
    NR_CONN_DNS
};
typedef struct site_s site_t;

struct site_s
{
/*the offical name of this site*/
    char *cname;
/* next call timstamp */
    time_t next_call;
/*TODO : ipv6 surpport*/
    struct sockaddr_in addr[MAX_ADDRS];
    char *host;
    enum dns_ste dns_state;
    bool in_ready_fifo; /*in dns ready fifo(dns ok and at least one url in site's fifo)?*/
    bool in_wait_fifo;   /*in dns wait fifo?*/
/* numbers of urls in ram for this site */
    uint16_t ram_urls;
/*the url list of this site,(the element of this fifo is string not url_t)*/
    fifo_t *url_fifo;
    /*already get the robots.txt*/
	bool robots_gotten;
    /* forbidden paths : given by robots.txt */
	char *forb_paths[MAX_FORB_PATHS];
};

site_t *site_new();
int site_init(site_t *si);
void site_destroy(site_t *si);

bool site_empty(site_t *si);
url_t *site_get_url(site_t *si);
void site_put_url(site_t *si, url_t *u);
/* return true if the path is allowed by robots.txt
   otherwith return false
 */
bool site_test_robots(site_t *si, char *path);
#endif
