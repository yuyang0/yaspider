#ifndef _URL_FIFO_H
#define _URL_FIFO_H
#define u_64 unsigned long long 

char *get_url_str (int priority);
void put_url_str (char *url_str, int prior);
size_t prior_url_num ();
size_t normal_url_num();
int init_url_fifo(char *prior_file, char *normal_file);
void destroy_url_fifo();
#endif
