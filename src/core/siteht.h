/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  siteht.h
 *        Created:  2013-04-14 17:18:58
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _SITEHT_H_
#define _SITEHT_H_ 1

void siteht_init();
site_t *siteht_get_site(char *host);
int siteht_set(char *host, site_t *s);
void siteht_release();

#endif /* _SITEHT_H_ */

