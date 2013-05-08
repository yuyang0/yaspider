/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  core.h
 *        Created:  2013-04-15 16:38:37
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _CORE_H_
#define _CORE_H_ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
/* other lib header files*/
//#include "db.h"
/*********end************/
#include "str.h"
#include "fifo.h"
#include "queue.h"
#include "wrapper.h"

#include "defines.h"

#include "conf.h"
#include "log.h"
#include "statistic.h"
#include "http.h"

#include "url_fifo.h"
#include "url.h"
#include "url_bf.h"
#include "site.h"
#include "siteht.h"
#include "connection.h"
#include "save.h"
#include "parse.h"
int init_all(char *conf_name);
void destroy_all();
void main_loop();
void fetch_urls();
void fetch_dns();
void fetch_pages();

void init_ev();

char *get_host(char *url_str);
#endif /* _CORE_H_ */

