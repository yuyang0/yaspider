#ifndef _BF_H_
#define _BF_H_
typedef struct bloom_filter_s bf_t;
struct bloom_filter_s
{
    unsigned char *bit_array;   /*the pointer to the bit array*/
    size_t n_bits;         /*the size of the bit array*/
    int n_hash;        /*the number of hash function*/
	size_t width;          /*bit width for the return value of the hash function*/
};

bf_t *bf_new (int, float);
int  bf_init(bf_t *, int, float); /*return -1 on error*/
void bf_reset (bf_t *bf);
void bf_destroy (bf_t *);
void bf_add(bf_t *, char *);
bool bf_test(bf_t *, char *);
bool bf_test_and_add (bf_t *, char *str);

#endif
