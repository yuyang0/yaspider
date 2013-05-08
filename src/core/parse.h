/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  parse.h
 *        Created:  2013-05-04 11:52:10
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _PARSE_H_
#define _PARSE_H_ 1

bool verify_url(char *url);
bool is_prior(char *url);
void fetch_all_urls(conn_t *cn);
int parse_robots(conn_t *cn);

#endif /* _PARSE_H_ */

