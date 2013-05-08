#ifndef _HTTP_H_
#define _HTTP_H_
#define HTTP_HDRS_MAX 256
typedef struct http_hdr_list_s  http_hdr_list_t;
typedef struct http_resp_s      http_resp_t;

struct http_hdr_list_s
{
  char *header[HTTP_HDRS_MAX];
  char *value[HTTP_HDRS_MAX];
};
struct http_resp_s
{
	float            http_ver;
	int              status_code;
	char            *reason_phrase;
	int              content_len;
	char            *body;
	http_hdr_list_t  *headers;
};
http_hdr_list_t *http_hdr_list_new();
void http_hdr_list_destroy(http_hdr_list_t *hdr_list);
char *http_hdr_list_get_value(http_hdr_list_t *hdr_list, char *name);
void http_hdr_list_set_value(http_hdr_list_t *hdr_list, char *name, char *value);

http_resp_t *http_resp_new();
int http_resp_init(http_resp_t *resp, char *buf, size_t len);
void http_resp_destroy(http_resp_t *resp);

#endif
