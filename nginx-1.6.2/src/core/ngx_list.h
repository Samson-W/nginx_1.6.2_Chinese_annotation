
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_LIST_H_INCLUDED_
#define _NGX_LIST_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct ngx_list_part_s  ngx_list_part_t;

struct ngx_list_part_s {
	//指向数组的起始地址
    void             *elts;
	//表示数组中已经使用了多少元素，大小必须小于ngx_list_t中的nalloc
    ngx_uint_t        nelts;
	//下一个链表元素ngx_list_part_t的地址
    ngx_list_part_t  *next;
};


typedef struct {
	//指向链表的最后一个数组元素
    ngx_list_part_t  *last;
	//指向链表的首个数组元素
    ngx_list_part_t   part;
	//链表中的每个ngx_list_part_t元素都是一个数组。size限制每一个数组元素的占用的空间大小
    size_t            size;
	//表示每个ngx_list_part_t数组的容量，即最多可存储多少个数据
    ngx_uint_t        nalloc;
	//链表中管理内存分配的内在池对象,用户要存放的数据占用的内存都是由pool分配的
    ngx_pool_t       *pool;
} ngx_list_t;

//创建新的链表
ngx_list_t *ngx_list_create(ngx_pool_t *pool, ngx_uint_t n, size_t size);

//初始化一个已有的链表
static ngx_inline ngx_int_t
ngx_list_init(ngx_list_t *list, ngx_pool_t *pool, ngx_uint_t n, size_t size)
{
    list->part.elts = ngx_palloc(pool, n * size);
    if (list->part.elts == NULL) {
        return NGX_ERROR;
    }

    list->part.nelts = 0;
    list->part.next = NULL;
    list->last = &list->part;
    list->size = size;
    list->nalloc = n;
    list->pool = pool;

    return NGX_OK;
}


/*
 *
 *  the iteration through the list:
 *
 *  part = &list.part;
 *  data = part->elts;
 *
 *  for (i = 0 ;; i++) {
 *
 *      if (i >= part->nelts) {
 *          if (part->next == NULL) {
 *              break;
 *          }
 *
 *          part = part->next;
 *          data = part->elts;
 *          i = 0;
 *      }
 *
 *      ...  data[i] ...
 *
 *  }
 */

//添加新的元素
void *ngx_list_push(ngx_list_t *list);


#endif /* _NGX_LIST_H_INCLUDED_ */
