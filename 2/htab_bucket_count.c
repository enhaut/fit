// htab_bucket_count.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "htab.h"
#include "htab_private.h"

size_t htab_bucket_count(const htab_t * t)
{
    if (!t)
        return 0;
    return t->arr_size;
}
