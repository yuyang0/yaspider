#include "core.h"
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/time.h>
#include <sys/resource.h>
/*
  #define _HAVE_SERVER_
*/
static void enable_coredump();

typedef void *(*THREAD_FUNC)(void *arg);
int start_thread(THREAD_FUNC fn, void *arg);
void *start_server(void *p_port);
typedef void (*Sigfunc) (int);

static void sigint_handler(int signo)
{
    exit(0);
}
/*the SEGV signal handler (self-debug)*/
/* static void sigsegv_handler(int signo) */
/* { */
/* 	void *trace[100]; */
/* 	char **messages = NULL; */
/*     int i, trace_size = 0; */
/*     trace_size = backtrace(trace, 100); */
/*     messages = backtrace_symbols(trace, trace_size); */
/*     log_error("segement fault"); */
/*     if (messages != NULL) */
/* 	    for (i = 0; i < trace_size; ++i) */
/* 	    	log_error("%s", messages[i]); */
/*     Free(messages); */
/* //    exit(-1); */
/*     abort(); /\* core dump*\/ */
/* } */

int main (int argc, char *argv[])
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        return -1;
    /* if (signal(SIGSEGV, sigsegv_handler) == SIG_ERR) */
    /* 	return -1; */
    enable_coredump();
    char *conf_dir = NULL;
    /* parse option */
    int opt;
    while ((opt = getopt(argc, argv, "c:h?")) != -1)
    {
        switch(opt)
        {
        case 'c':
            conf_dir = optarg;
            break;
        case 'h':
        case '?':
            printf ("-c: configure file path\n-h -?: show help\n");
            break;
        default:
            fprintf(stderr, "unknown option");
            break;
        }
    }
    if (init_all(conf_dir) < 0)
        exit(1);
    if (atexit(destroy_all) != 0)
    {
        log_fatal("can not register the destroy_all\n");
    }
    if (settings.have_server)
    {
        int err = start_thread (start_server, &settings.port);
        if (err != 0)
            log_error("start server error:%s", strerror(err));

    }

    printf("the spider is running...........\n");

    main_loop();
    return 0;
}

void enable_coredump()
{
    /*change the core dump resource limit*/
    struct rlimit rlim, rlim_new;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0)
    {
        rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &rlim_new)!= 0)
        {
            /* failed. try raising just to the old max */
            rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
            (void)setrlimit(RLIMIT_CORE, &rlim_new);
        }
    }
    /*
     * getrlimit again to see what we ended up with. Only fail if
     * the soft limit ends up 0, because then no core files will be
     * created at all.
     */

    if ((getrlimit(RLIMIT_CORE, &rlim) != 0) || rlim.rlim_cur == 0)
    {
        log_error("setrlimit core dump error");
        exit(1);
    }
}
