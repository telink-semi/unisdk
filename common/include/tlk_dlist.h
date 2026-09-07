#ifndef TLK_DLIST_H
#define TLK_DLIST_H

struct dnode
{
    struct dnode* next;
    struct dnode* prev;
};

#define TLK_DLIST_DEFINE(name)                                                                     \
    struct dnode name = {                                                                          \
        .next = &name,                                                                             \
        .prev = &name,                                                                             \
    }

#define TLK_DLIST_IS_EMPTY(list) ((list)->next == (list))

#define TLK_DLIST_FOR_EACH(list, item)                                                             \
    for ((item) = (list)->next; (item) != (list); (item) = (item)->next)

static _tlk_always_inline_ void tlk_dlist_init(struct dnode* list)
{
    list->next = list->prev = list;
}

static _tlk_always_inline_ void tlk_dlist_insert(struct dnode* node, struct dnode* prev,
                                                 struct dnode* next)
{
    node->next = next;
    node->prev = prev;
    prev->next = node;
    next->prev = node;
}

static _tlk_always_inline_ void tlk_dlist_append(struct dnode* list, struct dnode* node)
{
    tlk_dlist_insert(node, list->prev, list);
}

static _tlk_always_inline_ void tlk_dlist_prepend(struct dnode* list, struct dnode* node)
{
    tlk_dlist_insert(node, list, list->next);
}

static _tlk_always_inline_ void tlk_dlist_remove(struct dnode* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next       = NULL;
    node->prev       = NULL;
}

#endif /* __DLIST_H */
