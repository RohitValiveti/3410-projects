#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"

struct linkedlist
{
    struct linkedlist_node *first;
    unsigned int length;
};

struct linkedlist_node
{
    struct linkedlist_node *next;
    unsigned int key;
    unsigned int value;
};
typedef struct linkedlist_node linkedlist_node_t;

linkedlist_t *ll_init()
{
    linkedlist_t *list = malloc(sizeof(linkedlist_t));
    list->length = 0;
    list->first = NULL;
    return list;
}

void ll_free(linkedlist_t *list)
{
    linkedlist_node_t *ptr = list->first;
    while (ptr != NULL)
    {
        linkedlist_node_t *temp = ptr->next;
        free(ptr);
        ptr = temp;
    }
    free(list);
}

void ll_add(linkedlist_t *list, int key, int value)
{
    linkedlist_node_t *ptr = list->first;
    while (ptr != NULL)
    {
        if (ptr->key == key)
        {
            ptr->value = value;
            return;
        }
        else
        {
            ptr = ptr->next;
        }
    }
    linkedlist_node_t *newNode = malloc(sizeof(linkedlist_node_t));

    newNode->key = key;
    newNode->value = value;
    newNode->next = list->first;
    list->first = newNode;
    list->length++;
    return;
}

int ll_get(linkedlist_t *list, int key)
{
    linkedlist_node_t *ptr = list->first;
    while (ptr != NULL)
    {
        if (ptr->key == key)
        {
            return ptr->value;
        }
        else
        {
            ptr = ptr->next;
        }
    }
    return 0;
}

int ll_size(linkedlist_t *list)
{
    return list->length;
}
