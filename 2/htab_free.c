#include "htab.h"
#include "htab_private.h"


/** @brief Function destroys table.
 * @param t Pointer to the table
*/
void htab_free(htab_t * t)
{
    htab_clear(t);
    free(t);
}
