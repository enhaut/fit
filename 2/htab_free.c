// htab_free.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "htab.h"
#include "htab_private.h"


/** @brief Function destroys table.
 * @param t Pointer to the table
*/
void htab_free(htab_t * t)
{
    if (!t)
        return;

    htab_clear(t);
    free(t);
}
