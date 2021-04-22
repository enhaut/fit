// htab_lookup_add.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"


htab_item * get_last_item_of_index(htab_item *item)
{
    while(item)
    {
        if (item->next)
            item = item->next;
        else
            break;  // next item is NULL, so actual one is the last
    }
    return item;
}

/** @brief Function adds new pair to the table, in case key is already in table, it
 * just returns pointer to the existing one.
 * @param t Pointer to the table.
 * @param key Key of pair.
 * @return pointer to the (key, value) pair. In case, something fails it returns NULL;
*/
htab_pair_t * htab_lookup_add(htab_t * t, htab_key_t key)
{
    size_t key_size;
    if (!t || !key || !(key_size = strlen(key)))    // "" as a key is not allowed
        return NULL;

    size_t pair_index = htab_hash_function(key) % htab_bucket_count(t);
    if (t->data[pair_index])
    {
        htab_pair_t *pair = htab_find(t, key);
        if (pair)
            return pair;
    }

    char *key_copy = (char *)calloc(key_size + 1, sizeof(char));  // using calloc to add trailing \0
    if (!key_copy)
        return NULL;

    htab_item *new_item = (htab_item *)malloc(sizeof(htab_item));
    if (!new_item)
    {
        free(key_copy);
        return NULL;
    }

    strncpy(key_copy, key, key_size + 1);   // +1 to copy trailing \0
    new_item->element.key = key_copy;
    new_item->element.value = 0;
    new_item->next = NULL;

    if (t->data[pair_index])
    {
        htab_item *last_item = get_last_item_of_index(t->data[pair_index]);
        last_item->next = new_item;
    }else
        t->data[pair_index] = new_item;

    t->size++;

    return &(new_item->element);
}
