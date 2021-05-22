// htab_move.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "htab.h"
#include "htab_private.h"

/** @brief Function creates new hash table and moves values from old one.
 * After that, origin hash table stays empty but not reallocated.
 * @param n Size of new hash table.
 * @param from Table where values are moved from.
 * @returns pointer to the new hash table
*/
htab_t *htab_move(size_t n, htab_t *from)
{
    if (!from)
        return NULL;

    htab_t *to = htab_init(n);
    if (!to)
        return NULL;

    for (size_t i = 0; i < from->arr_size; i++)
    {
        if (!from->data[i])
            continue;

        htab_pair_t *moved_pair = htab_lookup_add(to, from->data[i]->element.key);
        moved_pair->value = from->data[i]->element.value;

        // move pairs at same index:
        htab_item *next_same_index_element = from->data[i]->next;
        while (next_same_index_element)
        {
            htab_pair_t * moved_pair2 = htab_lookup_add(to, next_same_index_element->element.key);
            moved_pair2->value = next_same_index_element->element.value;

            next_same_index_element = next_same_index_element->next;
        }
    }
    htab_clear(from);

    return to;
}
