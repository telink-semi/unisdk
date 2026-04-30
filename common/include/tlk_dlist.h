#ifndef TLK_DLIST_H
#define TLK_DLIST_H

#include "tlk_utils.h"

struct dnode {
    struct dnode *next;
    struct dnode *prev;
};

static inline void dlist_init(struct dnode *list)
{
    list->next = list->prev = list;
}

static inline void dlist_insert(struct dnode *node, struct dnode *prev, struct dnode *next)
{
    node->next = next;
    node->prev = prev;
    prev->next = node;
    next->prev = node;
}

static inline void dlist_append(struct dnode *list, struct dnode *node)
{
    dlist_insert(node, list->prev, list);
}

static inline void dlist_prepend(struct dnode *list, struct dnode *node)
{
    dlist_insert(node, list, list->next);
}

static inline void dlist_remove(struct dnode *node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
}

#define DLIST_DEFINE(name) \
    struct dnode name = { \
        .next = &name, \
        .prev = &name, \
    }

#define DLIST_IS_EMPTY(list) \
    ((list)->next == (list))

#define DLIST_FOR_EACH(list, item) \
    for ((item) = (list)->next; (item) != (list); (item) = (item)->next)

#endif /* __DLIST_H */
