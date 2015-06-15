
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_BUF_H_INCLUDED_
#define _NGX_BUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef void *            ngx_buf_tag_t;

typedef struct ngx_buf_s  ngx_buf_t;
//是nginx处理大数据的关键数据结构，既应用于内存数据也应用于磁盘数据
struct ngx_buf_s {
	//通常是用于告诉使用者本次应该从pos这个位置开始处理内存中的数据，这样设置是因为同一个ngx_buf_t可能被多次反复处理。pos的含义是由使用它的模块定义的
    u_char          *pos;
	//通常表示有效的内容到此为止。pos与last间的内存是希望nginx处理的内容
    u_char          *last;
	//处理文件时file_pos与file_last含义与处理内存时的pos与last相同，file_pos表示要处理的开始位置，而file_last表示截止的文件位置
    off_t            file_pos;
    off_t            file_last;
	//如果ngx_buf_t缓冲区用于内存，那么start指向这段内存的起始地址
    u_char          *start;         /* start of buffer */
	//与start成员对应，指向缓冲区内存的末尾
    u_char          *end;           /* end of buffer */
	//表示当前缓冲区的类型，例如由哪个模块使用就指向这个模块的ngx_module_t变量的地址
    ngx_buf_tag_t    tag;
	//引用的文件
    ngx_file_t      *file;
	//当前缓冲区的影子缓冲区，此成员很少用到，仅仅在12.8节描述的使用缓冲区转发上游服务器的响应时才使用了此成员，这是因为nginx太节约内存了，分配一块内存并使用ngx_buf_t表示接收到的上游服务器响应后，在向下游客户端转发时可能会把这块内存存储到文件中，也可能直接向下游发送，此时nginx绝不会重新复制一份内存用于新的目的，而是再次建立一个ngx_buf_t结构体指向原内存，这样多个ngx_buf_t结构体指向了同一块内存，它们之间的关系就通过shadow成员来引用。
    ngx_buf_t       *shadow;


    /* the buf's content could be changed */
	//为1时表示数据在内存中且这段内在可以修改
    unsigned         temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
	//为1时表示数据在内存中且这段内存不可能被修改
    unsigned         memory:1;

    /* the buf's content is mmap()ed and must not be changed */
	//为1时表示这段内存是用mmap系统调用映射过来的，不可以被修改
    unsigned         mmap:1;
	//为1时可回收
    unsigned         recycled:1;
	//表示这段缓冲区处理的是文件而不是内存
    unsigned         in_file:1;
	//为1时表示需要执行fluse操作
    unsigned         flush:1;
	//对于操作这块缓冲区时是否使用同步方式，需谨慎考虑，这可能会阻塞nginx进程，nginx中所有操作几乎都是异步的，这是它支持高并发的关键
    unsigned         sync:1;
	//为1时表示当前是最后一块待处理的缓冲区
    unsigned         last_buf:1;
	//为1时表示是否是ngx_chain_t中的最后一块缓冲区
    unsigned         last_in_chain:1;
	//为1时表示是否是最后个影子缓冲区，与shadow配合使用
    unsigned         last_shadow:1;
	//表示当前缓冲区是否属于临时文件 
    unsigned         temp_file:1;

    /* STUB */ int   num;
};

//是与ngx_buf_t配合使用的链表数据结构
struct ngx_chain_s {
	//指向当前的ngx_buf_t缓冲区
    ngx_buf_t    *buf;
	//指向下一个ngx)chain_t
    ngx_chain_t  *next;
};


typedef struct {
    ngx_int_t    num;
    size_t       size;
} ngx_bufs_t;


typedef struct ngx_output_chain_ctx_s  ngx_output_chain_ctx_t;

typedef ngx_int_t (*ngx_output_chain_filter_pt)(void *ctx, ngx_chain_t *in);

#if (NGX_HAVE_FILE_AIO)
typedef void (*ngx_output_chain_aio_pt)(ngx_output_chain_ctx_t *ctx,
    ngx_file_t *file);
#endif

struct ngx_output_chain_ctx_s {
	//保存临时的buf
    ngx_buf_t                   *buf;
	//保存了将要发送的chain
    ngx_chain_t                 *in;
	//保存了已经发送完毕的chain，以便于重复利用
    ngx_chain_t                 *free;
	//保存了还未发送的chain
    ngx_chain_t                 *busy;
	//sendfile标记
    unsigned                     sendfile:1;
	//directio标志
    unsigned                     directio:1;
#if (NGX_HAVE_ALIGNED_DIRECTIO)
    unsigned                     unaligned:1;
#endif
	//是否需要在内存中保存一份(使用sendfile的话，内存中没有文件的拷贝的，而我们有时需要处理文件，此时就需要设置这个标记)
    unsigned                     need_in_memory:1;
    unsigned                     need_in_temp:1;
	//是否需要在内存中重复复制一份，不管buf是在内存还是在文件，这样的话，后续模块可以直接修改这块内存
#if (NGX_HAVE_FILE_AIO)
    unsigned                     aio:1;

    ngx_output_chain_aio_pt      aio_handler;
#endif

    off_t                        alignment;

    ngx_pool_t                  *pool;
	//已经分配的buf个数
    ngx_int_t                    allocated;
	//对应loc conf中设置的bufs
    ngx_bufs_t                   bufs;
	//模块标记，主要用于buf回收
    ngx_buf_tag_t                tag;
	//一般是ngx_http_next_filter，也就是继续调用filter链
    ngx_output_chain_filter_pt   output_filter;
	//当前filter的上下文，这里是由于upstream也会调用output_chain
    void                        *filter_ctx;
};


typedef struct {
    ngx_chain_t                 *out;
    ngx_chain_t                **last;
    ngx_connection_t            *connection;
    ngx_pool_t                  *pool;
    off_t                        limit;
} ngx_chain_writer_ctx_t;


#define NGX_CHAIN_ERROR     (ngx_chain_t *) NGX_ERROR


#define ngx_buf_in_memory(b)        (b->temporary || b->memory || b->mmap)
#define ngx_buf_in_memory_only(b)   (ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_special(b)                                                   \
    ((b->flush || b->last_buf || b->sync)                                    \
     && !ngx_buf_in_memory(b) && !b->in_file)

#define ngx_buf_sync_only(b)                                                 \
    (b->sync                                                                 \
     && !ngx_buf_in_memory(b) && !b->in_file && !b->flush && !b->last_buf)

#define ngx_buf_size(b)                                                      \
    (ngx_buf_in_memory(b) ? (off_t) (b->last - b->pos):                      \
                            (b->file_last - b->file_pos))

ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
ngx_chain_t *ngx_create_chain_of_bufs(ngx_pool_t *pool, ngx_bufs_t *bufs);


#define ngx_alloc_buf(pool)  ngx_palloc(pool, sizeof(ngx_buf_t))
#define ngx_calloc_buf(pool) ngx_pcalloc(pool, sizeof(ngx_buf_t))

ngx_chain_t *ngx_alloc_chain_link(ngx_pool_t *pool);
#define ngx_free_chain(pool, cl)                                             \
    cl->next = pool->chain;                                                  \
    pool->chain = cl



ngx_int_t ngx_output_chain(ngx_output_chain_ctx_t *ctx, ngx_chain_t *in);
ngx_int_t ngx_chain_writer(void *ctx, ngx_chain_t *in);

ngx_int_t ngx_chain_add_copy(ngx_pool_t *pool, ngx_chain_t **chain,
    ngx_chain_t *in);
ngx_chain_t *ngx_chain_get_free_buf(ngx_pool_t *p, ngx_chain_t **free);
void ngx_chain_update_chains(ngx_pool_t *p, ngx_chain_t **free,
    ngx_chain_t **busy, ngx_chain_t **out, ngx_buf_tag_t tag);


#endif /* _NGX_BUF_H_INCLUDED_ */
