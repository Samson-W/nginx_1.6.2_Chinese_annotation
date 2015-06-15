
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_TIMES_H_INCLUDED_
#define _NGX_TIMES_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
	//格林威治时间1970.01.01 0：0：0到当前时间的秒数
    time_t      sec;
	//当前时间相对于sec的毫秒偏移量
    ngx_uint_t  msec;
	//时区
    ngx_int_t   gmtoff;
} ngx_time_t;


void ngx_time_init(void);
void ngx_time_update(void);
void ngx_time_sigsafe_update(void);
u_char *ngx_http_time(u_char *buf, time_t t);
u_char *ngx_http_cookie_time(u_char *buf, time_t t);
void ngx_gmtime(time_t t, ngx_tm_t *tp);

time_t ngx_next_time(time_t when);
#define ngx_next_time_n      "mktime()"

//ngx_time_t结构体形式的当前时间 
extern volatile ngx_time_t  *ngx_cached_time;

#define ngx_time()           ngx_cached_time->sec
#define ngx_timeofday()      (ngx_time_t *) ngx_cached_time
//用于记录error_log的当前时间字符串
extern volatile ngx_str_t    ngx_cached_err_log_time;
//用于http相关的当前时间字符串
extern volatile ngx_str_t    ngx_cached_http_time;
//用于记录http日志的当前时间字符串
extern volatile ngx_str_t    ngx_cached_http_log_time;
//以ISO 8601标准格式记录下的字符串形式的当前时间 
extern volatile ngx_str_t    ngx_cached_http_log_iso8601;

/*
 * milliseconds elapsed since epoch and truncated to ngx_msec_t,
 * used in event timers
 */
//格林威治时间1970.01.01 0：0：0到当前时间的毫秒数
extern volatile ngx_msec_t  ngx_current_msec;


#endif /* _NGX_TIMES_H_INCLUDED_ */
