#ifndef _URL_BF_H_
#define _URL_BF_H_
int url_bf_init(int num, float rate);
bool url_bf_test_and_add(char *url_str);
bool url_bf_test(char *url_str);/*return true if url_str in bf, otherwith return false*/
void url_bf_add(char *url_str);
void url_bf_reset();
#endif