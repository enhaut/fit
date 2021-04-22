// htab_size.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "htab.h"
#include "htab_private.h"

/** @brief Function returns size of table.
 * @param t Pointer to the table.
 * @returns Size of table. In case of invalid table, it returns 0.
*/
size_t htab_size(const htab_t * t)
{
    if (!t)
        return 0;
    return t->size;
}
