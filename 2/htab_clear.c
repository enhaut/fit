#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"

void htab_clear(htab_t * t)
{
    if (!t)
        return;

    for (size_t i = 0; i < t->arr_size; i++)
    {
        htab_element_t *next_same_index_element = t->data[i]->next;

        while (next_same_index_element)
        {
            htab_element_t *future_same_index_element = next_same_index_element->next;

            free(next_same_index_element);
            next_same_index_element = future_same_index_element;
        }
        free(t->data[i]);
        t->data[i] = NULL;
    }

    t->size = 1;
    t->arr_size = 0;
}
