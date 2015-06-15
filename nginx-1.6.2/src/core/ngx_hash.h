
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HASH_H_INCLUDED_
#define _NGX_HASH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
	//指向用户自定义元素数据的指针，若当前ngx_hash_elt_t槽为空，则value值为0
    void             *value;
	//元素关键字的长度
    u_short           len;
	//元素关键字的首地址
    u_char            name[1];
} ngx_hash_elt_t;


typedef struct {
	//指向散列表的首地址，也是第1个槽的地址
    ngx_hash_elt_t  **buckets;
	//散列表中槽的总数
    ngx_uint_t        size;
} ngx_hash_t;


typedef struct {
	//基本散列表
    ngx_hash_t        hash;
	//当使用这个ngx_hash_wildcard_t通配符散列表作为某容器的元素时，可以使用这个value指针指向用户数据
    void             *value;
} ngx_hash_wildcard_t;


typedef struct {
	//元素关键字
    ngx_str_t         key;
	//由散列方法算出来的关键码
    ngx_uint_t        key_hash;
	//指向实际的用户数据
    void             *value;
} ngx_hash_key_t;


typedef ngx_uint_t (*ngx_hash_key_pt) (u_char *data, size_t len);


typedef struct {
	//用于精确匹配的基本散列表
    ngx_hash_t            hash;
	//用于查询前置通配符的散列表
    ngx_hash_wildcard_t  *wc_head;
	//用于查询后置通配符的散列表
    ngx_hash_wildcard_t  *wc_tail;
} ngx_hash_combined_t;


typedef struct {
	//指向普通的完全匹配散列表
    ngx_hash_t       *hash;
	//用于初始化预添加元素的散列方法 
    ngx_hash_key_pt   key;
	//散列表中槽的最大数目
    ngx_uint_t        max_size;
	//散列表中一个槽的空间大小，它限制了每个散列表元素关键字的最大长度
    ngx_uint_t        bucket_size;
	//散列表的名称
    char             *name;
	//内存池，它分配散列表中的所有槽
    ngx_pool_t       *pool;
	//临时内存池，它仅存在于初始化散列表之前。主要用于分配一些临时的动态数组，带通配符的元素在初始化时需要用于这些数组
    ngx_pool_t       *temp_pool;
} ngx_hash_init_t;


#define NGX_HASH_SMALL            1
#define NGX_HASH_LARGE            2

#define NGX_HASH_LARGE_ASIZE      16384
#define NGX_HASH_LARGE_HSIZE      10007

#define NGX_HASH_WILDCARD_KEY     1
#define NGX_HASH_READONLY_KEY     2


typedef struct {
    ngx_uint_t        hsize;

    ngx_pool_t       *pool;
    ngx_pool_t       *temp_pool;

    ngx_array_t       keys;
    ngx_array_t      *keys_hash;

    ngx_array_t       dns_wc_head;
    ngx_array_t      *dns_wc_head_hash;

    ngx_array_t       dns_wc_tail;
    ngx_array_t      *dns_wc_tail_hash;
} ngx_hash_keys_arrays_t;

//此结构是为http头部量身订制的，其中key存储头部名称，value存储对应的值，lowcase_key是为了忽略http头部名称的大小写，hash 用于快速检索头部 3.6.3中
typedef struct {
	//表明ngx_table_elt_t也可以是某个散列表数据结构(ngx_hash_t类型)中的成员
    ngx_uint_t        hash;
	//名字
    ngx_str_t         key;
	//值字符串
    ngx_str_t         value;
	//指向的是全小写的key字符串
    u_char           *lowcase_key;
} ngx_table_elt_t;


void *ngx_hash_find(ngx_hash_t *hash, ngx_uint_t key, u_char *name, size_t len);
void *ngx_hash_find_wc_head(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_wc_tail(ngx_hash_wildcard_t *hwc, u_char *name, size_t len);
void *ngx_hash_find_combined(ngx_hash_combined_t *hash, ngx_uint_t key,
    u_char *name, size_t len);

ngx_int_t ngx_hash_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);
ngx_int_t ngx_hash_wildcard_init(ngx_hash_init_t *hinit, ngx_hash_key_t *names,
    ngx_uint_t nelts);

#define ngx_hash(key, c)   ((ngx_uint_t) key * 31 + c)
ngx_uint_t ngx_hash_key(u_char *data, size_t len);
ngx_uint_t ngx_hash_key_lc(u_char *data, size_t len);
ngx_uint_t ngx_hash_strlow(u_char *dst, u_char *src, size_t n);


ngx_int_t ngx_hash_keys_array_init(ngx_hash_keys_arrays_t *ha, ngx_uint_t type);
ngx_int_t ngx_hash_add_key(ngx_hash_keys_arrays_t *ha, ngx_str_t *key,
    void *value, ngx_uint_t flags);


#endif /* _NGX_HASH_H_INCLUDED_ */
