/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  conf.h
 *        Created:  2013-04-14 18:13:20
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _CONF_H_
#define _CONF_H_ 1

#define MAX_START_URLS 20
#define MAX_EXTS 128
#define MAX_DOMAIN 128
typedef struct _conf
{
    char *user_agent;             /* user-agent for this spider*/
    int port;                     /*the server port*/
    unsigned long max_ram_urls;       /*max number of urls allowed in ram*/
    int max_site_urls;                /*max number of urls per site*/
    int timeout;                   /*timeout of connection*/
    int interval;                  /*interval between two connection to a same server*/
    int max_conns;                 /*max number of connections*/
    int max_dns_calls;              /*max number of dns connections*/
    bool daemon;                  /*run as daemon?*/
    char *urls[MAX_START_URLS];   /*start urls*/
    char *prior_exts[MAX_EXTS];   /* priority extensions */
    char *forb_exts[MAX_EXTS];    /* forbidden extensions */
    char *allowed_domain[MAX_DOMAIN]; /* allowed domains, other domain not included will be forbidden */
    char *prior_domain[MAX_DOMAIN];
    int save;                   /* how to save the pages */
    char *save_path;              /* the path to save the files */

    bool have_server;
}conf_t;
extern conf_t settings;
int conf_init(char *path);
void conf_reset();
#endif /* _CONF_H_ */

