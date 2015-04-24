
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_UPSTREAM_H_INCLUDED_
#define _NGX_HTTP_UPSTREAM_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_event_connect.h>
#include <ngx_event_pipe.h>
#include <ngx_http.h>


#define NGX_HTTP_UPSTREAM_FT_ERROR           0x00000002
#define NGX_HTTP_UPSTREAM_FT_TIMEOUT         0x00000004
#define NGX_HTTP_UPSTREAM_FT_INVALID_HEADER  0x00000008
#define NGX_HTTP_UPSTREAM_FT_HTTP_500        0x00000010
#define NGX_HTTP_UPSTREAM_FT_HTTP_502        0x00000020
#define NGX_HTTP_UPSTREAM_FT_HTTP_503        0x00000040
#define NGX_HTTP_UPSTREAM_FT_HTTP_504        0x00000080
#define NGX_HTTP_UPSTREAM_FT_HTTP_403        0x00000100
#define NGX_HTTP_UPSTREAM_FT_HTTP_404        0x00000200
#define NGX_HTTP_UPSTREAM_FT_UPDATING        0x00000400
#define NGX_HTTP_UPSTREAM_FT_BUSY_LOCK       0x00000800
#define NGX_HTTP_UPSTREAM_FT_MAX_WAITING     0x00001000
#define NGX_HTTP_UPSTREAM_FT_NOLIVE          0x40000000
#define NGX_HTTP_UPSTREAM_FT_OFF             0x80000000

#define NGX_HTTP_UPSTREAM_FT_STATUS          (NGX_HTTP_UPSTREAM_FT_HTTP_500  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_502  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_503  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_504  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_403  \
                                             |NGX_HTTP_UPSTREAM_FT_HTTP_404)

#define NGX_HTTP_UPSTREAM_INVALID_HEADER     40


#define NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT    0x00000002
#define NGX_HTTP_UPSTREAM_IGN_XA_EXPIRES     0x00000004
#define NGX_HTTP_UPSTREAM_IGN_EXPIRES        0x00000008
#define NGX_HTTP_UPSTREAM_IGN_CACHE_CONTROL  0x00000010
#define NGX_HTTP_UPSTREAM_IGN_SET_COOKIE     0x00000020
#define NGX_HTTP_UPSTREAM_IGN_XA_LIMIT_RATE  0x00000040
#define NGX_HTTP_UPSTREAM_IGN_XA_BUFFERING   0x00000080
#define NGX_HTTP_UPSTREAM_IGN_XA_CHARSET     0x00000100


typedef struct {
    ngx_msec_t                       bl_time;
    ngx_uint_t                       bl_state;

    ngx_uint_t                       status;
    time_t                           response_sec;
    ngx_uint_t                       response_msec;
    off_t                            response_length;

    ngx_str_t                       *peer;
} ngx_http_upstream_state_t;


typedef struct {
    ngx_hash_t                       headers_in_hash;
    ngx_array_t                      upstreams;
                                             /* ngx_http_upstream_srv_conf_t */
} ngx_http_upstream_main_conf_t;

typedef struct ngx_http_upstream_srv_conf_s  ngx_http_upstream_srv_conf_t;

typedef ngx_int_t (*ngx_http_upstream_init_pt)(ngx_conf_t *cf,
    ngx_http_upstream_srv_conf_t *us);
typedef ngx_int_t (*ngx_http_upstream_init_peer_pt)(ngx_http_request_t *r,
    ngx_http_upstream_srv_conf_t *us);


typedef struct {
    ngx_http_upstream_init_pt        init_upstream;
    ngx_http_upstream_init_peer_pt   init;
    void                            *data;
} ngx_http_upstream_peer_t;


typedef struct {
    ngx_addr_t                      *addrs;
    ngx_uint_t                       naddrs;
    ngx_uint_t                       weight;
    ngx_uint_t                       max_fails;
    time_t                           fail_timeout;

    unsigned                         down:1;
    unsigned                         backup:1;
} ngx_http_upstream_server_t;


#define NGX_HTTP_UPSTREAM_CREATE        0x0001
#define NGX_HTTP_UPSTREAM_WEIGHT        0x0002
#define NGX_HTTP_UPSTREAM_MAX_FAILS     0x0004
#define NGX_HTTP_UPSTREAM_FAIL_TIMEOUT  0x0008
#define NGX_HTTP_UPSTREAM_DOWN          0x0010
#define NGX_HTTP_UPSTREAM_BACKUP        0x0020


struct ngx_http_upstream_srv_conf_s {
    ngx_http_upstream_peer_t         peer;
    void                           **srv_conf;

    ngx_array_t                     *servers;  /* ngx_http_upstream_server_t */

    ngx_uint_t                       flags;
    ngx_str_t                        host;
    u_char                          *file_name;
    ngx_uint_t                       line;
    in_port_t                        port;
    in_port_t                        default_port;
    ngx_uint_t                       no_port;  /* unsigned no_port:1 */
};


typedef struct {
    ngx_addr_t                      *addr;
    ngx_http_complex_value_t        *value;
} ngx_http_upstream_local_t;


typedef struct {
	//当在ngx_http_upstream_t结构体中没有实现resolved成员时，upstream结构体才会生效，它会定义上游服务器的配置
    ngx_http_upstream_srv_conf_t    *upstream;
	//以下3个超时时间是必须要设置的，因为它们默认为0,若不设置将永远无法与上游服务器建立起tcp连接
	//连接上游服务器的超时时间，单位为毫秒，实际上是写事件添加到定时器中时设置的超时时间  12.3第8步
    ngx_msec_t                       connect_timeout;
	//发送tcp包到上游服务器的超时时间，单位为毫秒，通常是写事件添加到定时器中设置的超时时间   12.4第3步
    ngx_msec_t                       send_timeout;
	//接收tcp包到上游服务器的超时时间，单位为毫秒，通常是读事件添加到定时器是设置的超时时间 12.4第5步
    ngx_msec_t                       read_timeout;
    ngx_msec_t                       timeout;
	//TCP的SO_SNOLOWAT选项，发送缓冲区的下限
    size_t                           send_lowat;
	//定义了接收头部的缓冲区分配的内在大小(ngx_http_upstream_t中的buffer缓冲区)，当不转发响应给下游或在buffering标志位为0的情况下转发响应时，同样表示接收包体的缓冲区大小
    size_t                           buffer_size;
	//仅当buffering标志位为1,且向下游转发响应时生效，它会设置到ngx_event_pipe_t结构体的busy_size成员中，具体含义12.8.1
    size_t                           busy_buffers_size;
	//在buffering标志位为1时，如果上游速度快于下游速度，将有可能把来自上游的响应存储到临时文件中，而max_temp_file_size指定了临时文件的最大长度。实际上，它将限制ngx_event_pipe_t结构体中的temp_file
    size_t                           max_temp_file_size;
	//表示将缓冲区中的响应写入临时文件时一次写入字符流的最大长度
    size_t                           temp_file_write_size;

    size_t                           busy_buffers_size_conf;
    size_t                           max_temp_file_size_conf;
    size_t                           temp_file_write_size_conf;
	//从缓存响应的方式转发上游服务器的包体时所使用的内存大小
    ngx_bufs_t                       bufs;
	//针对ngx_http_upstream_t结构体中保存解析完的包头的headers_in成员，ignore_headers要以按照二进制位使得upstream在转发包头时跳过某些头部的处理。NGX_HTTP_UPSTREAM_IGN_XA_REDIRECT 
    ngx_uint_t                       ignore_headers;
	//以二进制位来表示一些错误码，如果处理上游响应时发现这些错误码，那么在没有将响应转发给下游客户端时，将会选择下一个上游服务器来重发请求。 12.9中的ngx_http_upstream_next
    ngx_uint_t                       next_upstream;
	//在buffering标志位为1的情况下转发响应时，将有可能把响应存放到临时文件。在ngx_http_upstream_t中的store的标志位为1时，store_access表示所创建的目录、文件的权限
    ngx_uint_t                       store_access;
	//标记为1时，将开启更多的内存和磁盘文件用于缓存上游的响应包体；为0时将使用固定大小的缓冲大小
    ngx_flag_t                       buffering;
    ngx_flag_t                       pass_request_headers;
    ngx_flag_t                       pass_request_body;
	//为1,表示与上游服务器交互时将不检查nginx与下游客户端间的连接是否断开。也即是说，即使下游客户端主动关闭了连接，也不会中断与上游服务器间的交互
    ngx_flag_t                       ignore_client_abort;
	//当解析上游响应的包头时，如果解析后设置到headers_in结构体中的status_n错误码大于400,则会试图把它与error_page中指定的错误码相匹配，如果匹配上，则发送error_page中指定的响应，否则继续返回上游服务器的错误码。 ngx_http_upstream_intercept_errors
    ngx_flag_t                       intercept_errors;
	//buffering为1时转发响应时才有意义。这时，如果此值为1,则会试图复用临时文件中已经使用过的空间。不建议设置为1
    ngx_flag_t                       cyclic_temp_file;
	//在buffering标志位为1的情况下转发响应时，存放临时文件的路径
    ngx_path_t                      *temp_path;
	//不转发的头部。实际上是通过ngx_http_upstream_hide_headers_hase方法，根据hide_headers和pass_headers动态数组构造出的需要隐藏的http头部散列表
    ngx_hash_t                       hide_headers_hash;
	//当转发上游响应头部(ngx_http_upstream_t中headers_in结构体中的头部)给下游客户端时，如果不希望某些头部转发给下游，就设置到hide_headers动态数组中
    ngx_array_t                     *hide_headers;
	//当转发上游响应头部(ngx_http_upstream_t中headers_in结构体中的头部)给下游客户端时，upstream机制默认不会转发"Date" "Server"之类的头部，如果确实希望直接转发它们到下游，就设置到pass_headers动态数组中
    ngx_array_t                     *pass_headers;
	//连接上游服务器时使用的本机地址
    ngx_http_upstream_local_t       *local;

#if (NGX_HTTP_CACHE)
    ngx_shm_zone_t                  *cache;

    ngx_uint_t                       cache_min_uses;
    ngx_uint_t                       cache_use_stale;
    ngx_uint_t                       cache_methods;

    ngx_flag_t                       cache_lock;
    ngx_msec_t                       cache_lock_timeout;

    ngx_flag_t                       cache_revalidate;

    ngx_array_t                     *cache_valid;
    ngx_array_t                     *cache_bypass;
    ngx_array_t                     *no_cache;
#endif
	//当ngx_http_upstream_t中的store标志位为1时，如果需要将上游的响应存放到文件中，store_lengths将表示存放路径的长度，而store_values表示存放路径
    ngx_array_t                     *store_lengths;
    ngx_array_t                     *store_values;
	//与ngx_http_upstream_t中的store相同
    signed                           store:2;
	//上面的intercept_errors标志定义了400以上的错误码将会与error_page比较后再行处理，实际上这个规则是可以有一个例外情况的，如果将intercept_404标志设置为1,当上游返回404时会直接转发这个错误码给下游，而不会去与erro_page进行比较 
    unsigned                         intercept_404:1;
	//当该标志位为1时，将会根据ngx_http_upstream_t中headers_in结构体中的X-Accel-Buffering头部来改变buffering标志位，为1时将有可能根据上游服务器的响应头部，动态地决定是以上游网速优先还是以下游网速优先
    unsigned                         change_buffering:1;

#if (NGX_HTTP_SSL)
    ngx_ssl_t                       *ssl;
    ngx_flag_t                       ssl_session_reuse;
#endif
	//使用upstream的模块名称，仅用于记录日志 
    ngx_str_t                        module;
} ngx_http_upstream_conf_t;


typedef struct {
    ngx_str_t                        name;
    ngx_http_header_handler_pt       handler;
    ngx_uint_t                       offset;
    ngx_http_header_handler_pt       copy_handler;
    ngx_uint_t                       conf;
    ngx_uint_t                       redirect;  /* unsigned   redirect:1; */
} ngx_http_upstream_header_t;


typedef struct {
    ngx_list_t                       headers;

    ngx_uint_t                       status_n;
    ngx_str_t                        status_line;

    ngx_table_elt_t                 *status;
    ngx_table_elt_t                 *date;
    ngx_table_elt_t                 *server;
    ngx_table_elt_t                 *connection;

    ngx_table_elt_t                 *expires;
    ngx_table_elt_t                 *etag;
    ngx_table_elt_t                 *x_accel_expires;
    ngx_table_elt_t                 *x_accel_redirect;
    ngx_table_elt_t                 *x_accel_limit_rate;

    ngx_table_elt_t                 *content_type;
    ngx_table_elt_t                 *content_length;

    ngx_table_elt_t                 *last_modified;
    ngx_table_elt_t                 *location;
    ngx_table_elt_t                 *accept_ranges;
    ngx_table_elt_t                 *www_authenticate;
    ngx_table_elt_t                 *transfer_encoding;

#if (NGX_HTTP_GZIP)
    ngx_table_elt_t                 *content_encoding;
#endif

    off_t                            content_length_n;

    ngx_array_t                      cache_control;

    unsigned                         connection_close:1;
    unsigned                         chunked:1;
} ngx_http_upstream_headers_in_t;


typedef struct {
    ngx_str_t                        host;
    in_port_t                        port;
    ngx_uint_t                       no_port; /* unsigned no_port:1 */
	//地址个数
    ngx_uint_t                       naddrs;
    ngx_addr_t                      *addrs;
	//上游服务器地址
    struct sockaddr                 *sockaddr;
    socklen_t                        socklen;

    ngx_resolver_ctx_t              *ctx;
} ngx_http_upstream_resolved_t;


typedef void (*ngx_http_upstream_handler_pt)(ngx_http_request_t *r,
    ngx_http_upstream_t *u);


struct ngx_http_upstream_s {
	//处理读事件的回调方法，每一个阶段都有不同的read_event_handler
    ngx_http_upstream_handler_pt     read_event_handler;
	//处理写事件的回调方法，每一个阶段都有不同的write_event_handler
    ngx_http_upstream_handler_pt     write_event_handler;
	//表示主动向上游服务器发起的连接
    ngx_peer_connection_t            peer;
	//当向下游客户端转发响应时(ngx_http_request_t中的subrequest_in_memory标志位为0)，如果打开了缓存且认为上游网络更快(conf配置中的buffering标志位为1)，这时会使用pipe成员来转发响应。在使用这种方式转发响应时，必须由http模块在使用upstream机制前构造pipe结构体，否则会出现严重的coredump错误
    ngx_event_pipe_t                *pipe;
	//request_bufs以链表的方式把ngx_buf_t缓冲区链接起来，它表示所有需要发送到上游服务器的请求内容。所以，http模块实现的create_request回调方法就在于构建request_bufs链表
    ngx_chain_t                     *request_bufs;
	//定义了向下游发送响应的方式
    ngx_output_chain_ctx_t           output;
    ngx_chain_writer_ctx_t           writer;
	//upstream访问时的所有配置参数
    ngx_http_upstream_conf_t        *conf;
	//http模块在实现process_header方法时，如果希望upstream直接转发响应，就需要解析出的响应头部适配为http的响应头部，同时需要把包头中的信息设置到headers_in结构体中，这样，在图12-5的第8步中，会把headers_in中设置的头部添加到要发送到下游客户端的响应头部headers_out中
    ngx_http_upstream_headers_in_t   headers_in;
	//通过resolved可以直接指定上游服务器地址，用于解析主机域名
    ngx_http_upstream_resolved_t    *resolved;

    ngx_buf_t                        from_client;
	//buffer存储接收上游服务器发来的响应内容 P163  接收上游服务器响应包头的缓冲区，在不需要把响应直接转发给客户端，或buffering标志位为0的情况下转发包体时，接收包体的缓冲区仍然使用buffer。注意：如果没有自定义input_filter方法处理包体，将使用buffer存储全部的包体，这时buffer必须足够大。它的大小由ngx_http_upstream_conf_t配置结构中的buffer_size成员决定
    ngx_buf_t                        buffer;
	//表示来自上游服务器的响应包体的长度
    off_t                            length;
	//out_bufs在两种场景下有不同的意义：1 当不需要转发包体，且使用默认的intput_filter方法(ngx_http_upstream_non_buffered_filter)处理包体时，out_bufs将会指向响应包体，事实上，out_bufs链表中会产生多个ngx_buf_t缓冲区，每个缓冲区都指向buffer缓存中的一部分，而这里的一部分就是每次调用recv方法接收到的tcp流。 2 当需要转发响应包体到下游时(buffering标志位为0,即以下游戏网速优先，12.7),这个链表指向上一次向下游转发响应到现在这段时间内接收自上游戏的缓存响应
    ngx_chain_t                     *out_bufs;
	//当需要转发响应包体到下游时(buffering标志位为0 即以下游网速优先)，表示上一次向下游转发响应时没有发送完的内容 
    ngx_chain_t                     *busy_bufs;
	//这个链表用于回收out_bufs中已经发送给下游的ngx_buf_t结构体，这同样应用在buffering标志位为0即以下游网速优先的场景 
    ngx_chain_t                     *free_bufs;
	//处理上游的响应体  处理包体前的初始化方法，其中data用于传递用户数据结构，实际上就是下面的input_filter_ctx指针
    ngx_int_t                      (*input_filter_init)(void *data);
	//在转发上游响应到下游的同时，需要做一些特殊处理 当无须在上、下游间转发响应时，并不想等待接收完全部的上游响应后才开始处理请求时此方法进行调用 处理包体的方法，data是下面的input_filter_ctx指针  bytes表示本次接收到的包体长度，返回NGX_ERROR时表示处理包体错误，请求需要结束，否则都将继续upstream流程
    ngx_int_t                      (*input_filter)(void *data, ssize_t bytes);
	//用于传递http自定义的数据结构，在input_filter_init和input_filter方法被回调时会作为参数传递过去
    void                            *input_filter_ctx;

#if (NGX_HTTP_CACHE)
    ngx_int_t                      (*create_key)(ngx_http_request_t *r);
#endif
	//http模块实现的构造发往上游服务器的请求内容
    ngx_int_t                      (*create_request)(ngx_http_request_t *r);
	//与上游服务器的通信失败后，如果按照重试规则还需要再次向上游服务器发起连接，则会调用此接口
    ngx_int_t                      (*reinit_request)(ngx_http_request_t *r);
	//收到上游服务器的响应的包头。若process_header返回NGX_AGAIN，那么是在告诉upstream还没有收到完整的响应包头，此时，对于本次upstream请求来说，再次收到上游服务发来的tcp流时，还会调用process_header方法处理，直到process_header返回非NGX_AGAIN值这一阶段才会停止。
    ngx_int_t                      (*process_header)(ngx_http_request_t *r);
	//暂未使用
    void                           (*abort_request)(ngx_http_request_t *r);
	//请求结束时销毁upstream请求时调用 12.9.1
    void                           (*finalize_request)(ngx_http_request_t *r,
                                         ngx_int_t rc);
	//在上游戏返回的响应出现location或者refresh头部表示重定向时，会通过ngx_http_upstream_process_headers方法调用到可由http模块实现的rewrite_redirect方法
    ngx_int_t                      (*rewrite_redirect)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h, size_t prefix);
    ngx_int_t                      (*rewrite_cookie)(ngx_http_request_t *r,
                                         ngx_table_elt_t *h);

    ngx_msec_t                       timeout;
	//用于表示上游戏响应的错误码、包体长度等信息
    ngx_http_upstream_state_t       *state;
	//不使用文件缓存时没有意义
    ngx_str_t                        method;
	//以下二成员仅在记录日志时会用到
    ngx_str_t                        schema;
    ngx_str_t                        uri;
	//目前它仅用于表示是否需要清理资源，相当于一个标志位，实际不会调用到它所指向的方法
    ngx_http_cleanup_pt             *cleanup;
	//是否指定文件缓存路径的标志位
    unsigned                         store:1;
	//是否启用文件缓存
    unsigned                         cacheable:1;

    unsigned                         accel:1;
	//是否基于ssl协议访问上游服务器
    unsigned                         ssl:1;
#if (NGX_HTTP_CACHE)
    unsigned                         cache_status:3;
#endif
	//在向客户端转发上游服务器的包体时才有用。是否开启更大的内存及临时磁盘文件用于缓存来不及时发送到下游的响应包体。
    unsigned                         buffering:1;
    unsigned                         keepalive:1;
    unsigned                         upgrade:1;
	//是否已经向上游服务器发送了请求，当为1时，表示upstream机制已经向上游服务器发送了全部或者部分的请求。事实上，这个标志更多是为了使用ngx_output_chain方法发送请求，因为该方法发送请求时会自动把未发送完的request_bufs链表记录下来，为了防止反复发送重复请求，必须有request_sent标志位记录是否调用过ngx_output_chain方法
    unsigned                         request_sent:1;
	//将上游服务器的响应划分为包头和包尾，如果把响应直接转发给客户端，header_sent标志位表示包头是否发送，header_sent为1时表示已经把包头转发给客户端了。如果不转发响应到客户端，则header_sent没有意义 
    unsigned                         header_sent:1;
};


typedef struct {
    ngx_uint_t                      status;
    ngx_uint_t                      mask;
} ngx_http_upstream_next_t;


typedef struct {
    ngx_str_t   key;
    ngx_str_t   value;
    ngx_uint_t  skip_empty;
} ngx_http_upstream_param_t;


ngx_int_t ngx_http_upstream_header_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);

ngx_int_t ngx_http_upstream_create(ngx_http_request_t *r);
void ngx_http_upstream_init(ngx_http_request_t *r);
ngx_http_upstream_srv_conf_t *ngx_http_upstream_add(ngx_conf_t *cf,
    ngx_url_t *u, ngx_uint_t flags);
char *ngx_http_upstream_bind_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
char *ngx_http_upstream_param_set_slot(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
ngx_int_t ngx_http_upstream_hide_headers_hash(ngx_conf_t *cf,
    ngx_http_upstream_conf_t *conf, ngx_http_upstream_conf_t *prev,
    ngx_str_t *default_hide_headers, ngx_hash_init_t *hash);


#define ngx_http_conf_upstream_srv_conf(uscf, module)                         \
    uscf->srv_conf[module.ctx_index]


extern ngx_module_t        ngx_http_upstream_module;
extern ngx_conf_bitmask_t  ngx_http_upstream_cache_method_mask[];
extern ngx_conf_bitmask_t  ngx_http_upstream_ignore_headers_masks[];


#endif /* _NGX_HTTP_UPSTREAM_H_INCLUDED_ */
