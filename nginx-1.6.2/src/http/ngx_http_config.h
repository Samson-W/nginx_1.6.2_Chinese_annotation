
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_CONFIG_H_INCLUDED_
#define _NGX_HTTP_CONFIG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
	//指针数组，数组中的每个元素指向所有http模块reate_main_conf方法产生的结构体
    void        **main_conf;
	//指针数组，数组中的每个元素指向所有http模块的create_svr_conf方法产生的结构体
    void        **srv_conf;
	//指针数组，数组中的每个元素指向所有http模块的create_loc_conf方法产生的结构体
    void        **loc_conf;
} ngx_http_conf_ctx_t;


typedef struct {
	//解析配置文件前调用
    ngx_int_t   (*preconfiguration)(ngx_conf_t *cf);
	//完成配置文件的解析后调用
    ngx_int_t   (*postconfiguration)(ngx_conf_t *cf);
	//当需要创建数据结构用于存储main级别的(直属于http{}块)的全局配置项时，通过此回调创建存储全局配置项的结构体
    void       *(*create_main_conf)(ngx_conf_t *cf);
	//用于初始化main级别配置项
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
	//当需要创建数据结构用于存储srv级别(直属于虚拟主机server{}块配置)的配置项时，可以通过此回调方法创建存储srv级别配置项的结构体
    void       *(*create_srv_conf)(ngx_conf_t *cf);
	//此回调方法主要用于合并main级别和srv级别下的同名配置项,cf提供一些基本的数据结构，如内存池，日志。我们需要关注的是第2 3个参数，其中第2个参数void *prev是指解析父配置块时生成的结构体，而第3个参数void *conf则指出的是保存子配置块的结构体。
    char       *(*merge_srv_conf)(ngx_conf_t *cf, void *prev, void *conf);
	//当需要创建数据结构体用于存储loc级别(直属于location{}块的配置项)的配置项时，可以实现此回调
    void       *(*create_loc_conf)(ngx_conf_t *cf);
	//此回调方法主要用于合并srv有别和loc级别下的同名配置项
    char       *(*merge_loc_conf)(ngx_conf_t *cf, void *prev, void *conf);
} ngx_http_module_t;


#define NGX_HTTP_MODULE           0x50545448   /* "HTTP" */
//配置项可以出现在http{}块内
#define NGX_HTTP_MAIN_CONF        0x02000000
//配置项可以出现在server{}块内，然而该server块必须属于http{}块
#define NGX_HTTP_SRV_CONF         0x04000000
//配置项可以出现在location{}块内，然而该location块必须属于http块
#define NGX_HTTP_LOC_CONF         0x08000000
//配置项可以出现在upstream{}块内。然而该upstream块必须属于http块
#define NGX_HTTP_UPS_CONF         0x10000000
//配置项可以出现在location块内的if{}块中。目前仅有rewrite模块会使用，该if块必须属于http{}块
#define NGX_HTTP_SIF_CONF         0x20000000
//配置项可以出现在location块内的if{}块中。目前仅有rewrite模块会使用，该if块必须属于http{}块
#define NGX_HTTP_LIF_CONF         0x40000000
//配置项可以出现在limit_except{}块内，然而该limit_except块必须属于http{}块
#define NGX_HTTP_LMT_CONF         0x80000000

//使用create_main_conf方法产生的结构体来存储解析出的配置项参数
#define NGX_HTTP_MAIN_CONF_OFFSET  offsetof(ngx_http_conf_ctx_t, main_conf)
//使用create_srv_conf方法产生的结构体来存储解析出的配置项参数
#define NGX_HTTP_SRV_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, srv_conf)
//使用create_loc_conf方法产生的结构体来存储解析出的配置项参数
#define NGX_HTTP_LOC_CONF_OFFSET   offsetof(ngx_http_conf_ctx_t, loc_conf)


#define ngx_http_get_module_main_conf(r, module)                             \
    (r)->main_conf[module.ctx_index]
#define ngx_http_get_module_srv_conf(r, module)  (r)->srv_conf[module.ctx_index]
#define ngx_http_get_module_loc_conf(r, module)  (r)->loc_conf[module.ctx_index]


#define ngx_http_conf_get_module_main_conf(cf, module)                        \
    ((ngx_http_conf_ctx_t *) cf->ctx)->main_conf[module.ctx_index]
#define ngx_http_conf_get_module_srv_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->srv_conf[module.ctx_index]
#define ngx_http_conf_get_module_loc_conf(cf, module)                         \
    ((ngx_http_conf_ctx_t *) cf->ctx)->loc_conf[module.ctx_index]

#define ngx_http_cycle_get_module_main_conf(cycle, module)                    \
    (cycle->conf_ctx[ngx_http_module.index] ?                                 \
        ((ngx_http_conf_ctx_t *) cycle->conf_ctx[ngx_http_module.index])      \
            ->main_conf[module.ctx_index]:                                    \
        NULL)


#endif /* _NGX_HTTP_CONFIG_H_INCLUDED_ */
