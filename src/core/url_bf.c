#include <stdio.h>
#include <stdbool.h>
#include "bf.h"
#include "url_bf.h"

static bf_t url_bf;

int url_bf_init(int num, float rate)
{
	return bf_init(&url_bf, num, rate);
}
bool url_bf_test_and_add(char *url_str)
{
	return bf_test_and_add(&url_bf, url_str);
}
/*return true if url_str in bloom filter, otherwith return false*/
bool url_bf_test(char *url_str)
{
	return bf_test(&url_bf, url_str);
}
void url_bf_add(char *url_str)
{
	bf_add(&url_bf, url_str);
}
void url_bf_reset()
{
	bf_reset(&url_bf);
}
