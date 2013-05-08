#include "core.h"

/*the global state of the spider*/
struct statistic statis;

int init_all(char *conf_dir)
{
    char log_conf[MAXLINE];
    char settings_conf[MAXLINE];
    /* dirs we will try */
    char *dirs[] = {
        "../conf",
        "conf",
        "../../conf",
        conf_dir,
        NULL
    };
    char **ptr = dirs;
    while(*ptr)
    {
        strncpy(log_conf, *ptr, MAXLINE);
        strncpy(settings_conf, *ptr, MAXLINE);
        strcpy(log_conf + strlen(log_conf), "/zlog.conf");
        strcpy(settings_conf + strlen(settings_conf), "/settings.conf");
        if (log_init(log_conf) >= 0 &&
            conf_init(settings_conf) >= 0)
        {
            break;
        }
        ptr++;
    }
    if (*ptr == NULL)
    {
        return -1;
    }
	/*16000000个url且误差为0.01*/
	if (url_bf_init(16000000, 0.01) < 0)
		return -1;
	/*init the global url fifo variable(used as url fifo)*/
	if (init_url_fifo(NULL, NULL) < 0)
		return -1;
    siteht_init();
    if (init_conns() < 0)
    	return -1;
    init_ev();
	int i = 0;
	while (settings.urls[i])
	{
        url_bf_add(settings.urls[i]);
        put_url_str(settings.urls[i], 1);
		++i;
	}
    fetch_urls();
    fetch_dns();
	return 0;
}
void destroy_all ()
{
	printf("destroying all sources\n");
	log_destroy();
	destroy_url_fifo();
	url_bf_reset();
    siteht_release();
    destroy_conns();
    conf_reset();
}
