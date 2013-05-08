#ifndef _LOG_H_
#define _LOG_H_ 
#include "zlog.h"
enum {
	ZLOG_LEVEL_OK_URL      = 2,
	ZLOG_LEVEL_SKIPPED_URL = 3,
	ZLOG_LEVEL_TMOUT_URL   = 4,
	ZLOG_LEVEL_ERROR_URL   = 5,
  };
extern zlog_category_t *sys_cat;
extern zlog_category_t *url_cat;
/*the macros for system errors*/
#define log_fatal(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_FATAL, __VA_ARGS__)

#define log_error(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define log_warn(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_WARN, __VA_ARGS__)

#define log_notice(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define log_info(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_INFO, __VA_ARGS__)

#define log_debug(...) \
	zlog(sys_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__, \
	     ZLOG_LEVEL_DEBUG, __VA_ARGS__)
/*the macros for url errors*/

#define log_ok_url(...)   \
	zlog(url_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__,  \
		 ZLOG_LEVEL_OK_URL, __VA_ARGS__)

#define log_skipped_url(...)   \
	zlog(url_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__,  \
		 ZLOG_LEVEL_SKIPPED_URL, __VA_ARGS__)

#define log_timeout_url(...)   \
	zlog(url_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__,  \
		 ZLOG_LEVEL_TMOUT_URL, __VA_ARGS__)

#define log_error_url(...)   \
	zlog(url_cat, __FILE__, sizeof(__FILE__) - 1, __func__, sizeof(__func__) - 1, __LINE__,  \
		 ZLOG_LEVEL_ERROR_URL, __VA_ARGS__)

int  log_init(char *confpath);
void log_destroy();
#endif