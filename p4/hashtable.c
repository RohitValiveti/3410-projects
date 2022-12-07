#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "hashtable.h"

struct hashtable
{
    linkedlist_t **array;
    unsigned int numBuckets;
};

/**
 * Hash function to hash a key into the range [0, max_range)
 */
static int hash(int key, int max_range)
{
    key = (key > 0) ? key : -key;
    return key % max_range;
}

hashtable_t *ht_init(int num_buckets)
{
    hashtable_t *hshtbl = malloc(sizeof(hashtable_t));
    hshtbl->numBuckets = num_buckets;
    hshtbl->array = malloc(num_buckets * sizeof(linkedlist_t *));

    for (int i = 0; i < num_buckets; i++)
    {
        hshtbl->array[i] = ll_init();
    }

    return hshtbl;
}

void ht_free(hashtable_t *table)
{
    for (int i = 0; i < table->numBuckets; i++)
    {
        ll_free(table->array[i]);
    }
    free(table->array);
    free(table);
}

void ht_add(hashtable_t *table, int key, int value)
{
    int bucketIdx = hash(key, table->numBuckets);
    ll_add(table->array[bucketIdx], key, value);
}

int ht_get(hashtable_t *table, int key)
{
    int bucketIdx = hash(key, table->numBuckets);
    return ll_get(table->array[bucketIdx], key);
}

int ht_size(hashtable_t *table)
{
    int mappings = 0;
    for (int i = 0; i < table->numBuckets; i++)
    {
        mappings += ll_size(table->array[i]);
    }
    return mappings;
}
