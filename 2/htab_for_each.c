// htab_for_each.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "htab.h"
#include "htab_private.h"

/** @brief Function creates new hash table and moves values from old one.
 * After that, origin hash table stays empty but not reallocated.
 * @param t hash table.
 * @param f function that is called for every item in table.
*/
void htab_for_each(const htab_t * t, void (*f)(htab_pair_t *data))
{
    if (!t || !f)
        return;

    for (size_t i = 0; i < t->arr_size; i++)
    {
        if (!t->data[i])
            continue;

        f(&(t->data[i]->element));  // user in worst scenario can free(key), but that's not my business

        htab_item *same_index_item = t->data[i]->next;
        while (same_index_item)
        {
            f(&(same_index_item->element));
            same_index_item = same_index_item->next;
        }
    }
}
