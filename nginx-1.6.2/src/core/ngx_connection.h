
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CONNECTION_H_INCLUDED_
#define _NGX_CONNECTION_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_listening_s  ngx_listening_t;
//P270
struct ngx_listening_s {
	//socket套接字句柄
    ngx_socket_t        fd;
	//监听sockaddr地址
    struct sockaddr    *sockaddr;
	//sockaddr地址长度
    socklen_t           socklen;    /* size of sockaddr */
	//存储IP地址的字符串addr_text最大长度
    size_t              addr_text_max_len;
	//以字符串形式存储IP地址
    ngx_str_t           addr_text;
	//套接字类型
    int                 type;
	//tcp实现监听时的backlog队列，表示允许正在通过3次握手建立tcp连接但还没有任何进程开始处理的连接最大数
    int                 backlog;
	//内核中对于这个套接字的接收缓冲区大小
    int                 rcvbuf;
	//内核中对于这个套接字的发送缓冲区大小
    int                 sndbuf;
#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    int                 keepidle;
    int                 keepintvl;
    int                 keepcnt;
#endif

    /* handler of accepted connection */
	//当新的tcp连接成功建立后的处理方法
    ngx_connection_handler_pt   handler;
	//在http mail模块，用于保存当前监听端口对应着的所有主机名
    void               *servers;  /* array of ngx_http_in_addr_t, for example */
	//两个都是可用的日志对象的指针
    ngx_log_t           log;
    ngx_log_t          *logp;
	//如果为新的tcp连接创建内存池，则内存池的初始大小应该是pool_size
    size_t              pool_size;
    /* should be here because of the AcceptEx() preread */
    size_t              post_accept_buffer_size;
    /* should be here because of the deferred accept */
    ngx_msec_t          post_accept_timeout;
	//前一个ngx_listening_t结构
    ngx_listening_t    *previous;
	//当前监听名柄对应着的结构
    ngx_connection_t   *connection;

    unsigned            open:1;
    unsigned            remain:1;
    unsigned            ignore:1;

    unsigned            bound:1;       /* already bound */
    unsigned            inherited:1;   /* inherited from previous process */
    unsigned            nonblocking_accept:1;
    unsigned            listen:1;
    unsigned            nonblocking:1;
    unsigned            shared:1;    /* shared between threads or processes */
    unsigned            addr_ntop:1;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    unsigned            ipv6only:1;
#endif
    unsigned            keepalive:2;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    unsigned            deferred_accept:1;
    unsigned            delete_deferred:1;
    unsigned            add_deferred:1;
#ifdef SO_ACCEPTFILTER
    char               *accept_filter;
#endif
#endif
#if (NGX_HAVE_SETFIB)
    int                 setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    int                 fastopen;
#endif

};


typedef enum {
     NGX_ERROR_ALERT = 0,
     NGX_ERROR_ERR,
     NGX_ERROR_INFO,
     NGX_ERROR_IGNORE_ECONNRESET,
     NGX_ERROR_IGNORE_EINVAL
} ngx_connection_log_error_e;


typedef enum {
     NGX_TCP_NODELAY_UNSET = 0,
     NGX_TCP_NODELAY_SET,
     NGX_TCP_NODELAY_DISABLED
} ngx_connection_tcp_nodelay_e;


typedef enum {
     NGX_TCP_NOPUSH_UNSET = 0,
     NGX_TCP_NOPUSH_SET,
     NGX_TCP_NOPUSH_DISABLED
} ngx_connection_tcp_nopush_e;


#define NGX_LOWLEVEL_BUFFERED  0x0f
#define NGX_SSL_BUFFERED       0x01
#define NGX_SPDY_BUFFERED      0x02


struct ngx_connection_s {
	//连接未使用时，data成员用于充当连接池中空闲连接链表中的next指针。当连接被使用时，data的意义由使用它的nginx模块而定，如在http模块中，data指向ngx_http_request_t请求
    void               *data;
	//连接对应的读事件 
    ngx_event_t        *read;
	//连接对应的写事件 
    ngx_event_t        *write;
	//套接字句柄
    ngx_socket_t        fd;
	//直接接收网络字符流的方法
    ngx_recv_pt         recv;
	//直接发送网络字符流的方法
    ngx_send_pt         send;
	//以ngx_chain_t链表为参数来接收网络字符流的方法
    ngx_recv_chain_pt   recv_chain;
	//以ngx_chain_t链表为参数来发送网络字符流的方法
    ngx_send_chain_pt   send_chain;
	//这个连接对应的ngx_listening_t监听对象，此连接由listenting监听端口的事件建立 
    ngx_listening_t    *listening;
	//这个连接上已经发送出去的字节数
    off_t               sent;
	//可以记录上场的ngx_log对象
    ngx_log_t          *log;
	//内在池。一般在accept一个新连接时，会创建一个内存池，而在这个连接结束时会销毁内存池。这里所说的连接是指成功建立的tcp连接，所有ngx_connection_t结构体都是预分配的。这个内存池的大小将由上面的listening监听对象中的pool_size成员决定
    ngx_pool_t         *pool;
	//sockaddr结构体的长度
    struct sockaddr    *sockaddr;
	//sockaddr结构体的长度
    socklen_t           socklen;
	//连接客户端字符串形式的IP地址
    ngx_str_t           addr_text;
	//本机的监听端口对应的sockaddr结构体，也就是listening监听对象中的sockaddr成员
    ngx_str_t           proxy_protocol_addr;

#if (NGX_SSL)
    ngx_ssl_connection_t  *ssl;
#endif

    struct sockaddr    *local_sockaddr;
    socklen_t           local_socklen;
	//用于接收、缓存客户端发来的字符流，每个事件消费模块可自由决定从连接池中分配多大的空间给buffer这个接收缓存字段。
    ngx_buf_t          *buffer;
	//用于将当前连接以双向链表元素的形式添加到ngx_cycle_t核心结构的reusable_connections_queue双向链表中，表示可以重用的连接
    ngx_queue_t         queue;
	//连接使用次数。ngx_connection_t结构体每次建立一条来自客户端的连接，或用于主动向后端服务器发起连接时(ngx_peer_connection_t也使用它)，number都会加1
    ngx_atomic_uint_t   number;
	//处理的请求次数
    ngx_uint_t          requests;
	//缓存中的业务类型
    unsigned            buffered:8;
	//本连接记录日志时的级别，占用3位，
    unsigned            log_error:3;     /* ngx_connection_log_error_e */
	//为1时表示不期待字符流结束
    unsigned            unexpected_eof:1;
	//为1时表示连接已经超时
    unsigned            timedout:1;
	//为1时表示连接处理过程中出现错误
    unsigned            error:1;
	//为1时表示连接已经销毁。这里的连接指是的tcp连接，而不是ngx_connection_t结构体。当为1时，ngx_connection_t结构体仍然存在，但其对应的套接字、内存池已经不可用
    unsigned            destroyed:1;
	//为1时表示连接处于空闲状态
    unsigned            idle:1;
	//为1表示连接可用，与queue是对应使用的
    unsigned            reusable:1;
	//为1表示连接关闭
    unsigned            close:1;
	//为1时表示正在将文件中的数据发往连接的另一端
    unsigned            sendfile:1;
	//若为1,表示只有连接套接字对应的发送缓冲区必须满足最低设置的大小阀值时，事件驱动模块才会分发该事件。这与上文中的ngx_handle_wirte_event方法中的lowat参数是对应的
    unsigned            sndlowat:1;
	//表示如何使用tcp的nodelay特性
    unsigned            tcp_nodelay:2;   /* ngx_connection_tcp_nodelay_e */
	//表示如何使用tcp的nopush特性
    unsigned            tcp_nopush:2;    /* ngx_connection_tcp_nopush_e */

    unsigned            need_last_buf:1;

#if (NGX_HAVE_IOCP)
    unsigned            accept_context_updated:1;
#endif

#if (NGX_HAVE_AIO_SENDFILE)
	//为1时表示使用异步IO的方式将磁盘上文件发送给网络连接的另一端
    unsigned            aio_sendfile:1;
    unsigned            busy_count:2;
	//使用异步IO方式发送的文件，busy_sendfile缓冲区保存待发送文件的信息
    ngx_buf_t          *busy_sendfile;
#endif

#if (NGX_THREADS)
    ngx_atomic_t        lock;
#endif
};


ngx_listening_t *ngx_create_listening(ngx_conf_t *cf, void *sockaddr,
    socklen_t socklen);
ngx_int_t ngx_set_inherited_sockets(ngx_cycle_t *cycle);
ngx_int_t ngx_open_listening_sockets(ngx_cycle_t *cycle);
void ngx_configure_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_listening_sockets(ngx_cycle_t *cycle);
void ngx_close_connection(ngx_connection_t *c);
ngx_int_t ngx_connection_local_sockaddr(ngx_connection_t *c, ngx_str_t *s,
    ngx_uint_t port);
ngx_int_t ngx_connection_error(ngx_connection_t *c, ngx_err_t err, char *text);

ngx_connection_t *ngx_get_connection(ngx_socket_t s, ngx_log_t *log);
void ngx_free_connection(ngx_connection_t *c);

void ngx_reusable_connection(ngx_connection_t *c, ngx_uint_t reusable);

#endif /* _NGX_CONNECTION_H_INCLUDED_ */
