#include <stdlib.h>
#include "htab.h"
#include "htab_private.h"


/** @brief Function initializes hash table.
 * @param n Table elements.
 * @return pointer to the initialized hash table
*/
htab_t *htab_init(size_t n)
{
    htab_t *table = (htab_t *)malloc(sizeof(htab_t) + sizeof(htab_item *) * n);
    if (!table)
        return NULL;
    else
    {
        table->size = n + 1;
        table->arr_size = n;
        for (; n; n--)
            table->data[n - 1] = NULL;  // -1 because of indexing correction
    }
    return table;
}
