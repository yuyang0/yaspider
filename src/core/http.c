#include "core.h"
#include <ctype.h>
#include <zlib.h>
#include "zstream.h"
/***private fuction declaration**********************************/
static int parse_resp(http_resp_t *resp, char *buf, size_t len);

static int merge_chunk(char *src);
static int get_resp_body(http_resp_t *resp, char *body_start, size_t len);
static int get_resp_hdr(http_resp_t *resp, char *hdr_start);
/************end*************************************************/
http_hdr_list_t *http_hdr_list_new()
{
	http_hdr_list_t *ptr = calloc(sizeof(*ptr), 1);
	if (ptr == NULL)
		log_fatal("calloc error ");
	return ptr;
}

void http_hdr_list_destroy(http_hdr_list_t *hdr_list)
{
	assert(hdr_list);
    int i;
    for (i = 0; i < HTTP_HDRS_MAX && hdr_list->header[i]; i++)
	{
		free(hdr_list->header[i]);
        
        assert(hdr_list->value[i]);
		free(hdr_list->value[i]);
	}

	free(hdr_list);
}

char *http_hdr_list_get_value(http_hdr_list_t *hdr_list, char *name)
{
	assert(hdr_list && name);
	int i = 0;
	while(i < HTTP_HDRS_MAX && hdr_list->header[i] != NULL)
	{
		if (!strcasecmp(hdr_list->header[i], name))
			return hdr_list->value[i];
		i++;
	}
	return NULL;

}
void http_hdr_list_set_value(http_hdr_list_t *hdr_list, char *name, char *value)
{
	assert(hdr_list && name && value);
	int i = 0;
    /*find a null entry*/
	while (hdr_list->header[i] != NULL && i < HTTP_HDRS_MAX)
    {
        i++;
    }
	if (i >= HTTP_HDRS_MAX - 1)
	{
		log_warn("too many headers");
		return;
	}
	hdr_list->header[i] = strdup(name);
	hdr_list->value[i] = strdup(value);

    if (!hdr_list->header[i] || !hdr_list->value[i])
    {
        log_error("strdup error");
        hdr_list->header[i] = NULL;
        return;
    }
	hdr_list->header[++i] = NULL;
}

http_resp_t *http_resp_new()
{
	http_resp_t *resp = calloc(sizeof(*resp), 1);
	if (!resp)
		log_fatal("malloc error");
	resp->headers = http_hdr_list_new();
	return resp;
}

/*
  parse the buffer and init the response structure with
  header list and body
*/
int http_resp_init(http_resp_t *resp, char *buf, size_t len)
{
    if (!buf)
    {
        log_error("http_resp_init: buf is NULL");
        return -1;
    }
    if (parse_resp(resp, buf, len) < 0)
        return -1;
    else
        return 0;
}
void http_resp_destroy(http_resp_t *resp)
{
	if (resp == NULL)
		return;
	if (resp->headers)
		http_hdr_list_destroy(resp->headers);
	if(resp->reason_phrase)
		free(resp->reason_phrase);
    if (resp->body)
        free(resp->body);
	free(resp);
}

static int parse_resp(http_resp_t *resp, char *buf, size_t len)
{
    char *hdr_list_end = strstr(buf, "\r\n\r\n");
    if (!hdr_list_end)
    {
        log_error("parse_resp: header error");
        return -1;
    }
    char *body_start = hdr_list_end + 4;
    size_t body_len = len - (body_start - buf);
    
    if (get_resp_hdr(resp, buf) < 0)
    	return -1;
    char *val = http_hdr_list_get_value(resp->headers, "Content-Type");
    if (val)
    {
        if (!strcasestr(val, "text/html") &&
            !strcasestr(val, "text/plain"))
        {
            return -1;
        }
    }
    if (body_len > 0)
    {
        if (get_resp_body(resp, body_start, body_len) < 0)
            return -1;
    }
    return 0;
}

static int get_resp_hdr(http_resp_t *resp, char *hdr_start)
{
    char *line_start = hdr_start;
	char *line_end = strstr(hdr_start, "\r\n");
	if (line_end == NULL)
		goto header_error;
	/*get the status code*/
	if (!isdigit(line_start[9]) ||
        !isdigit(line_start[10]) ||
        !isdigit(line_start[11]))
    {
        goto header_error;
    }
	resp->status_code = ((line_start[9] - 0x30)*100);
    resp->status_code += ((line_start[10] - 0x30)*10);
    resp->status_code += (line_start[11] - 0x30);
	/*get the header list*/
	char *hdr_list_start = line_end + 2;
	char *hdr_list_end = strstr(hdr_start, "\r\n\r\n");
	if (hdr_list_end == NULL)
		goto header_error;
	line_start = hdr_list_start;
	while(!startswith(line_start, "\r\n"))
	{
		line_end = strstr(line_start, "\r\n");
		if (line_end == NULL)
			goto header_error;
		*line_end = '\0';
		char *line_middle = strchr(line_start, ':');
		if (line_middle == NULL || line_middle >= line_end)
			goto header_error;
		*line_middle = '\0';
		/*get header entry name and value(must strip the space at start and end)*/
		char *hdr_entry_name = strip(line_start, " ");
		char *hdr_entry_value = strip(line_middle + 1, " ");
		http_hdr_list_set_value(resp->headers, hdr_entry_name, hdr_entry_value);
		line_start = line_end + 2;
	}
    return 0;
header_error:
	log_error("get_resp_hdr: header error");
	return -1;
	
}
static int get_resp_body(http_resp_t *resp, char *body_start, size_t len)
{
    assert(resp && body_start);
	bool is_chunked = false;
    char *val = http_hdr_list_get_value(resp->headers, "Transfer-Encoding");
    if (val && strcasestr(val, "chunked"))
    {
        is_chunked = true;
    }
	int content_len;
	if (is_chunked)
	{
		content_len = merge_chunk(body_start);
	}
	else
	{
        val = http_hdr_list_get_value(resp->headers, "Content-length");
        if (val)
        {
            content_len = atoi(val);
            if (content_len > len)
            {
                content_len = len;
            }
        }
        else
        {
            content_len = len;  // use the data length
        }
	}
	/*the buffer end with '\0'*/
	val = http_hdr_list_get_value(resp->headers, "Content-Encoding");
	bool is_compressed = false;
	if (val && strcasestr(val, "gzip"))
	{
		is_compressed = true;
	}
	if (is_compressed)
	{
	    resp->body = Malloc(content_len * 6);
	    Byte *data = (Byte *)(resp->body);
	    uLong ndata = content_len * 6;
	    if (httpgzdecompress((Byte *)body_start, content_len, data, &ndata) < 0)
        {
            log_error("gzip decompress error");
            return -1;
        }
	    resp->content_len = ndata;
	}
	else
	{
        /*content_len+1 is a safer size than content_len*/
		resp->body = Malloc(content_len + 1);
       
	    memcpy(resp->body, body_start, content_len);
	    resp->content_len = content_len;
	}
	return 0;
}


static int htoi(char *str)
 {
 	int idx = 0;
 	int result = 0;
 	if (str[0] == '0' && ((str[1] | 32)== 'x'))
 		idx = 2;
 	while ((str[idx] >= '0' && str[idx] <= '9') || ((str[idx] | 32) >= 'a' && (str[idx] | 32) <='f'))
 	{
 		if (str[idx] > '9')
 			result = result * 16 + (str[idx] | 32) - 'a' + 10;
 		else
 			result = result * 16 + str[idx] - '0';
 		idx++;
 	}
 	return result;
 }
 /*原地合并chunk，并返回合并后的数据大小*/
static int merge_chunk(char *src)
{
    assert(src);
	int chunk_size;
	char *chunk_start, *chunk_end;
	char *data_start;
	char *buf = src;
	chunk_start = src;
	while(1)
	{
	    char *ptr = strstr(chunk_start, "\r\n");
	    if (ptr == NULL)
	    {
	    	log_error("a error in chunked data");
	    	return -1;
	    }
	    *ptr = '\0';
	    chunk_size = htoi(chunk_start);
	    if (chunk_size == 0)
	    	break;
	    data_start = ptr + 2;
	    char *data_end = data_start + chunk_size;
	    chunk_end = data_start + chunk_size + 2;
	    char *data = data_start;
	    while (data < data_end)
	    	*buf++ = *data++;
    	*buf = '\0';
	    chunk_start = chunk_end;
	}
	return (buf - src);

}
