
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_RADIX_TREE_H_INCLUDED_
#define _NGX_RADIX_TREE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#define NGX_RADIX_NO_VALUE   (uintptr_t) -1

typedef struct ngx_radix_node_s  ngx_radix_node_t;

struct ngx_radix_node_s {
	//批向右子树，若没有右子树，则值为NULL空指针
    ngx_radix_node_t  *right;
	//指向左子树，若没有左子树，则值为NULL空指针
    ngx_radix_node_t  *left;
	//指向父节点，若没有父节点，则值为NULL
    ngx_radix_node_t  *parent;
	//存储的是指针的值，它指向用户自定义的数据结构。若这个节点还未使用，此值为NGX_RADIX_NO_VALUE
    uintptr_t          value;
};


typedef struct {
	//指向根节点
    ngx_radix_node_t  *root;
	//内存池，它负责给基数树的节点分配内存
    ngx_pool_t        *pool;
	//管理已经分配但暂时未使用的节点
    ngx_radix_node_t  *free;
	//已分配内存中还未使用内存的首地址
    char              *start;
	//已分配内存中还未使用的内存大小
    size_t             size;
} ngx_radix_tree_t;


ngx_radix_tree_t *ngx_radix_tree_create(ngx_pool_t *pool,
    ngx_int_t preallocate);

ngx_int_t ngx_radix32tree_insert(ngx_radix_tree_t *tree,
    uint32_t key, uint32_t mask, uintptr_t value);
ngx_int_t ngx_radix32tree_delete(ngx_radix_tree_t *tree,
    uint32_t key, uint32_t mask);
uintptr_t ngx_radix32tree_find(ngx_radix_tree_t *tree, uint32_t key);

#if (NGX_HAVE_INET6)
ngx_int_t ngx_radix128tree_insert(ngx_radix_tree_t *tree,
    u_char *key, u_char *mask, uintptr_t value);
ngx_int_t ngx_radix128tree_delete(ngx_radix_tree_t *tree,
    u_char *key, u_char *mask);
uintptr_t ngx_radix128tree_find(ngx_radix_tree_t *tree, u_char *key);
#endif


#endif /* _NGX_RADIX_TREE_H_INCLUDED_ */
