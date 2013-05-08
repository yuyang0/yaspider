
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#ifndef _QUEUE_H_INCLUDED_
#define _QUEUE_H_INCLUDED_
typedef struct queue_s queue_t;

struct queue_s {
    queue_t  *prev;   /*前一个*/
    queue_t  *next;   /*下一个*/
};

/*初始化队列 */ 
#define queue_init(q)                                                     \
    (q)->prev = q;                                                            \
    (q)->next = q

/*判断队列是否为空*/
#define queue_empty(h)                                                    \
    (h == (h)->prev)

/*在头节点之后插入新节点*/
#define queue_insert_head(h, x)                                           \
    (x)->next = (h)->next;                                                    \
    (x)->next->prev = x;                                                      \
    (x)->prev = h;                                                            \
    (h)->next = x

#define queue_insert_after   queue_insert_head

/*在尾节点之后插入新节点*/
#define queue_insert_tail(h, x)                                           \
    (x)->prev = (h)->prev;                                                    \
    (x)->prev->next = x;                                                      \
    (x)->next = h;                                                            \
    (h)->prev = x

/*头节点*/
#define queue_head(h)                                                     \
    (h)->next

/*尾节点*/
#define queue_last(h)                                                     \
    (h)->prev

/*头部标志节点*/
#define queue_sentinel(h)                                                 \
    (h)

/*下一个节点*/
#define queue_next(q)                                                     \
    (q)->next

/*上一个节点*/
#define queue_prev(q)                                                     \
    (q)->prev


#if (DEBUG)

/*删除节点*/
#define queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next;                                              \
    (x)->prev = NULL;                                                         \
    (x)->next = NULL

#else

#define queue_remove(x)                                                   \
    (x)->next->prev = (x)->prev;                                              \
    (x)->prev->next = (x)->next

#endif

/*分隔队列*/
#define queue_split(h, q, n)                                              \
    (n)->prev = (h)->prev;                                                    \
    (n)->prev->next = n;                                                      \
    (n)->next = q;                                                            \
    (h)->prev = (q)->prev;                                                    \
    (h)->prev->next = h;                                                      \
    (q)->prev = n;

/*链接队列*/
#define queue_add(h, n)                                                   \
    (h)->prev->next = (n)->next;                                              \
    (n)->next->prev = (h)->prev;                                              \
    (h)->prev = (n)->prev;                                                    \
    (h)->prev->next = h;

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &(((TYPE *)0)->MEMBER))
#endif
/*获取队列中节点数据， q是队列中的节点，type队列类型，link是队列类型中queue_t的元素名*/
#define queue_data(q, TYPE, MEMBER)                                         \
    (TYPE *) ((unsigned char *) q - offsetof(TYPE, MEMBER))

/*队列的中间节点*/
/*
queue_t *queue_middle(queue_t *queue);
void queue_sort(queue_t *queue,
    int (*cmp)(const queue_t *, const queue_t *));

*/

#endif /* _QUEUE_H_INCLUDED_ */
