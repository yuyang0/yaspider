/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  save.h
 *        Created:  2013-04-28 12:46:55
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _SAVE_H_
#define _SAVE_H_ 1
enum SAVE
{
    SAVE_FILE,
    SAVE_MIRROR,
    SAVE_DB
};
int save(conn_t *cn);

#endif /* _SAVE_H_ */

