#include <stdio.h>

#include "zlog.h"
#include "log.h"
zlog_category_t *url_cat;
zlog_category_t *sys_cat;
int log_init(char *confpath)
{
	int rc;

    rc = zlog_init(confpath);
    if (rc) 
    {
        return -1;
    }
    url_cat = zlog_get_category("url_category");
    sys_cat = zlog_get_category("sys_category");
    if (!url_cat || !sys_cat) 
    {
        fprintf(stderr, "get cat fail\n");
        zlog_fini();
        return -2;
    }
    return 0;
}
void log_destroy()
{
	zlog_fini();
}

#if(0)
int main(int argc, char const *argv[])
{
    if(log_init("../conf/zlog.conf") < 0)
        return -1;
    char *url[] = {
        "http://news.sina.com.cn/z/zhonggongshibada/",
        "http://login.sina.com.cn/",
        "http://mail.sina.com.cn/",
        "http://weibo.com/",
        "http://blog.sina.com.cn/u/1891379660",
        "http://you.video.sina.com.cn/m/1891379660",
        "http://bbs.sina.com.cn/",
        "http://login.sina.com.cn/",
        "http://login.sina.com.cn/member/security/password.php",
        "https://login.sina.com.cn/sso/logout.php",
        "http://weibo.com/",
        "http://login.sina.com.cn/",
        "http://help.sina.com.cn/index.php",
        "http://mail.sina.net/",
        "http://sina.cn/",
        "http://news.sina.com.cn/guide/",
        "http://news.sina.com.cn/",
        "http://mil.news.sina.com.cn/",
        "http://news.sina.com.cn/society/",
        NULL
    };
    char **ptr = url;
    while(*ptr)
    {
        log_ok_url("hello world %d:%s", 1, *ptr);
        log_skipped_url("hello world %d:%s", 1, *ptr);
        log_timeout_url("hello world %d:%s", 1, *ptr);
        log_error_url("hello world %d:%s", 1, *ptr);

        log_info("hello world %d:%s", 1, *ptr);
        log_debug("hello world %d:%s", 1, *ptr);
        log_error(*ptr);
        log_warn(*ptr);
        ptr++;
    }
    log_destroy();
    return 0;
}
#endif