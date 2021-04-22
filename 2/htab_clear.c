// htab_clear.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"


/** @brief Function clears all the data from table but
 * table is not reallocated.
 * @param t Pointer to the table.
*/
void htab_clear(htab_t * t)
{
    if (!t)
        return;

    for (size_t i = 0; i < t->arr_size; i++)
    {
        if (!t->data[i])
            continue;

        htab_item *next_same_index_element = t->data[i]->next;

        while (next_same_index_element)
        {
            htab_item *future_same_index_element = next_same_index_element->next;

            free((void *)(next_same_index_element->element.key));
            free(next_same_index_element);
            next_same_index_element = future_same_index_element;
        }
        free((void *)(t->data[i]->element.key));
        free(t->data[i]);
        t->data[i] = NULL;
    }

    t->size = 0;
}
