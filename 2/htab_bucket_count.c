#include "htab.h"
#include "htab_private.h"

size_t htab_bucket_count(const htab_t * t)
{
    if (!t)
        return 0;
    return t->arr_size;
}
