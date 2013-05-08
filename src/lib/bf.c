#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "wrapper.h"
#include "bf.h"

#define CHAR_SIZE sizeof(char)

#define set_bit(a, n)     *((a) + n/CHAR_SIZE) |= (1 << (n % CHAR_SIZE))
#define get_bit(a, n)     *((a) + n/CHAR_SIZE) & (1 << (n % CHAR_SIZE))


static uint32_t hashmurmur3_32(const void *data, size_t nbytes);

bf_t *bf_new (int num, float rate)
{
	bf_t *bf = Malloc (sizeof (*bf));
	if (bf_init (bf, num, rate) < 0)
	{
		Free (bf);
		return NULL;
	}
	return bf;
}
int bf_init(bf_t *bf, int num, float rate)
{
	int i = 1;
	size_t m = -(num * log(rate))/(log(2)*log(2));     /*位数组的长度*/
	while (m > (1 << i))                               /*位数组尽量为2的幂*/
		i++;
	if (i < 16)                                        /*最小16位*/
		i = 16;
	m = (1<<i);
	int k = log(2) * m / num;                        /*hash函数的个数*/
	bf->bit_array = malloc(m/CHAR_SIZE);
	
	if (bf->bit_array != NULL)
	{
		memset((void*)(bf->bit_array), 0, m/CHAR_SIZE);
		bf->n_bits = m;
		bf->n_hash = k;
		bf->width = i;
		return 0;
	}
	else
		return -1;
}
void bf_reset (bf_t *bf)
{
	if (bf == NULL)
		return;
	Free(bf->bit_array);
}
void bf_destroy (bf_t *bf)
{
	if (bf == NULL)
		return;
	Free(bf->bit_array);
	Free (bf);
}

bool bf_test(bf_t *bf, char *str)
{
	uint32_t hash_res;
	size_t pos;
	unsigned int tmp;
	char suffix;
	/*fixme:the url maybe greater than 4096*/
	char p[4096];
	int len = strlen(str) + 3;
	memcpy(p, str, len - 3);
	p[len - 3] = '_';
	int i;
	for (i = 0; i < bf->n_hash; ++i)
	{
		suffix = i + 48;
		p[len - 2] = suffix;
		p[len - 1] = '\0';
       
        hash_res = hashmurmur3_32(p, strlen(p));
		/*将32位的结果转换为指定的位数，eg：32位转24位，将后24位与高8位取异或（^)*/
		pos = (hash_res >> bf->width) ^ (hash_res &((1 << bf->width) - 1));
		tmp = get_bit(bf->bit_array, pos);
		if (tmp == 0)
			return false;
	}
	return true;
}

void bf_add(bf_t *bf, char *str)
{
	uint32_t hash_res;
	size_t pos;
	char suffix;
	int len = strlen(str) + 3;
	char *p = malloc(len);
	memcpy(p, str, len - 3);
	int i;
	for (i = 0; i < bf->n_hash; ++i)
	{
		suffix = i + 48;
		p[len - 3] = '_';
		p[len - 2] = suffix;
		p[len - 1] = '\0';
        
        hash_res = hashmurmur3_32(p, strlen(p));
		pos = (hash_res >> bf->width) ^ (hash_res &((1 << bf->width) - 1));
		set_bit(bf->bit_array, pos);
	}
}
/*是否在集合中，如果在返回true，如果不在返回false，并将该值加入集合*/
bool bf_test_and_add (bf_t *bf, char *str)
{
	if (bf_test(bf, str) == false)
	{
		bf_add(bf, str);
		return false;
	}
	else
		return true;
}

/*thr murmur hash function*/
static uint32_t hashmurmur3_32(const void *data, size_t nbytes)
{
    if (data == NULL || nbytes == 0) return 0;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    const int nblocks = nbytes / 4;
    const uint32_t *blocks = (const uint32_t *)(data);
    const uint8_t *tail = (const uint8_t *)(data + (nblocks * 4));

    uint32_t h = 0;

    int i;
    uint32_t k;
    for (i = 0; i < nblocks; i++) {
        k = blocks[i];

        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;

        h ^= k;
        h = (h << 13) | (h >> (32 - 13));
        h = (h * 5) + 0xe6546b64;
    }

    k = 0;
    switch (nbytes & 3) {
        case 3:
            k ^= tail[2] << 16;
        case 2:
            k ^= tail[1] << 8;
        case 1:
            k ^= tail[0];
            k *= c1;
            k = (k << 13) | (k >> (32 - 15));
            k *= c2;
            h ^= k;
    };

    h ^= nbytes;

    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}
