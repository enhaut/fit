// htab_erase.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include <string.h>
#include "htab.h"
#include "htab_private.h"

/** @brief Function removes one element from table.
 * @param t Pointer to the table
 * @param key key of pair to delete
 * @returns true in case of success otherwise false
*/
bool htab_erase(htab_t * t, htab_key_t key)
{
    if (!t || !key || !strlen(key)) // "" as a key is not allowed
        return false;

    size_t pair_index = htab_hash_function(key) % htab_bucket_count(t);

    if (!t->data[pair_index])   // key is not in the table
        return false;

    htab_item * to_erase = t->data[pair_index];
    if (to_erase->next)
    {
        while (to_erase)
        {
            size_t max_key_size = strlen(to_erase->element.key);
            if (strncpy((char *)to_erase->element.key, key, max_key_size) == 0)
                break;  // key found

            to_erase = to_erase->next;
        }
    }else
    {
        size_t max_key_size = strlen(to_erase->element.key);
        if (strncmp((char *)to_erase->element.key, key, max_key_size) != 0)
            return false;   // key for erase is at the same index, but keys does not match (conflict)

        t->data[pair_index] = NULL; // no item remains
    }

    t->size--;
    free((void *)(to_erase->element.key));  // deallocating memory for key
    free(to_erase); // deallocating memory for item
    return true;
}
