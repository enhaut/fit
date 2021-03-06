// htab_find.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include <string.h>
#include "htab.h"
#include "htab_private.h"

/** @brief Function finds pair (key, value) in the table
 * and returns pointer to that.
 * @param t Pointer to the table.
 * @param key Key that function is looking for.
 * @return pointer to the (key, value) pair. In case no table is provided, it returns NULL.
*/
htab_pair_t * htab_find(htab_t * t, htab_key_t key)
{
    if (!t)
        return NULL;

    size_t item_index = htab_hash_function(key) % t->arr_size;
    htab_item *item = t->data[item_index];
    if (!item)
        return NULL;

    // at the same index could be more than 1 pair, so we need to find the right one
    if (item->next)
    {
        while (item)
        {
            size_t saved_key_size = strlen(item->element.key);
            if (strncmp(item->element.key, key, saved_key_size) == 0)
                break;  // wanted pair found
            item = item->next;
        }
    }else
        if (strncmp(item->element.key, key, strlen(item->element.key)) != 0)
            return NULL; // item is at the same position but that could be coincidence

    if (!item)  // key at index of hash table not found
        return NULL;

    return &(item->element);
}
