#include "tlk_utils.h"
#include <stddef.h>

struct tlk_list_node
{
    struct tlk_list_node* next;
};

struct tlk_linked_list
{
    struct tlk_list_node* head;
};

#define TLK_LIST_FOR_EACH(list, pos, type, member)                                                 \
    for (pos = (list)->head ? TLK_CONTAINER_OF((list)->head, type, member) : NULL; pos != NULL;    \
         pos = pos->member.next ? TLK_CONTAINER_OF(pos->member.next, type, member) : NULL)

static _tlk_always_inline_ void tlk_list_init(struct tlk_linked_list* list)
{
    list->head = NULL;
}

static _tlk_always_inline_ void tlk_list_prepend(struct tlk_linked_list* list,
                                                 struct tlk_list_node*   node)
{
    node->next = list->head;
    list->head = node;
}
