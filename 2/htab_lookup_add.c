#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

/** @brief Function adds new pair to the table, in case key is already in table, it
 * just returns pointer to the existing one.
 * @param t Pointer to the table.
 * @param key Key of pair.
 * @return pointer to the (key, value) pair. In case, something fails it returns NULL;
*/
htab_pair_t * htab_lookup_add(htab_t * t, htab_key_t key)
{
    size_t key_size;
    if (!t || !key || !(key_size = strlen(key)))
        return NULL;

    size_t pair_index = htab_hash_function(key) % htab_bucket_count(t);

    if (t->data[pair_index])
        return htab_find(t, key);

    char *key_copy = (char *)malloc(sizeof(char) * key_size + 1);
    if (!key_copy)
        return NULL;

    htab_item *new_item = (htab_item *)malloc(sizeof(htab_item));
    if (!new_item)
    {
        free(key_copy);
        return NULL;
    }

    strncpy(key_copy, key, key_size);
    new_item->element.key = key_copy;
    new_item->element.value = 0;
    new_item->next = NULL;

    t->data[pair_index] = new_item;

    return &(new_item->element);
}
