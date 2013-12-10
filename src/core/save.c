/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  save.c
 *        Created:  2013-04-28 12:48:16
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "core.h"
#define MAX_FILES_PER_DIR 3000

/* private function */
static int Mkdir(const char *path, mode_t mode);
static int recur_mkdir(char *path, mode_t mode);

static int save_file(conn_t *cn);
static int save_mirror(conn_t *cn);
static int save_db(conn_t *cn);
/* end */
int save(conn_t *cn)
{
    switch(settings.save)
    {
    case SAVE_FILE:
        save_file(cn);
        break;
    case SAVE_MIRROR:
        save_mirror(cn);
        break;
    case SAVE_DB:
        save_db(cn);
        break;
    default:
        /* do nothing*/
        break;
    }
    return 0;
}
/* simple save, MAX_FILES_PER_DIR files per directory */
static int save_file(conn_t *cn)
{
    static size_t dir_idx, file_idx;
    char path[MAXLINE];
    memset(path, 0, MAXLINE);
    snprintf(path, MAXLINE, "%sd%09u", settings.save_path, dir_idx);
    if (file_idx == 0)
    {
        if (Mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
        {
            log_error("can not make directory: %s", path);
            return -1;
        }
    }
    sprintf(path + strlen(path), "/%u", file_idx);
    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        log_error("fopen error %s", path);
        return -1;
    }
    char *body = cn->resp->body;
    fwrite(body, 1, cn->resp->content_len, fp);
    fclose(fp);
    file_idx++;
    if (file_idx == MAX_FILES_PER_DIR)
    {
        file_idx = 0;
        dir_idx++;
    }
    return 0;
}
/* save as mirror of the site */
static int save_mirror(conn_t *cn)
{
    char path[MAXLINE];
    memset(path, 0, MAXLINE);
    snprintf(path, MAXLINE, "%s%s%s", settings.save_path, cn->url->host, cn->url->path);
    /* get dirname and make it if the directory is not exist */
    char *p = strrchr(path, '/');
    assert(p);
    *p = '\0';
    if (recur_mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR) < 0)
        return -1;
    /* append the filename to the path */
    if (*(p + 1) == '\0')
    {
        /* the url's path ends with '/' */
        strcpy(path + strlen(path), "/home");
    }
    else
    {
        *p = '/';
    }
    /* open the file and write the content to it */
    FILE *fp = fopen(path, "w");
    if (!fp)
    {
        log_error("fopen error %s", path);
        return -1;
    }
    char *body = cn->resp->body;
    fwrite(body, 1, cn->resp->content_len, fp);
    fclose(fp);
    return 0;
}
static int save_db(conn_t *cn)
{
    return 0;
}

static int recur_mkdir(char *path, mode_t mode)
{
    char *pp;
    char *sp;
    int  status;
    char copypath[MAXLINE];
    strncpy(copypath, path, MAXLINE);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != NULL)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = Mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = Mkdir(path, mode);
    return (status);
}

static int Mkdir(const char *path, mode_t mode)
{
    struct stat st;
    int         status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}
