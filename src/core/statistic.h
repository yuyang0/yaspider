#ifndef _STATISTIC_H_
#define _STATISTIC_H_


struct statistic
{
    int ram_urls;      //ok
    int ok_urls;       //ok
    int error_urls;    //ok
    int skipped_urls;  //ok

    int ram_sites;      //ok
    int dns_ok_sites;   //ok
    int dns_err_sites;  //ok

    int dns_calls;       //ok
    int conns;

};
extern struct statistic statis;
#endif /* _STATISTIC_H_ */
