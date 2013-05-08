/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  connection.h
 *        Created:  2013-04-09 12:43:42
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _CONNECTION_H_
#define _CONNECTION_H_ 1

struct _conn
{
    site_t       *site;
    url_t        *url;

    char         *buf;    //io buffer
    size_t       size;    //the buffer size
    size_t       dsize;    //data size

    http_resp_t  *resp;
    queue_t      queue;
};
typedef struct _conn conn_t;

int init_conns();
conn_t *get_free_conn();
void stop_conn(conn_t *conn);
void destroy_conns();
void conn_append(conn_t *cn, char *data, size_t data_len); //append data to connection buffer
#endif /* _CONNECTION_H_ */

