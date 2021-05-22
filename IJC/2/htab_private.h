// htab_private.h
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Privatne štruktúry

#include <stdlib.h>
#include "htab.h"

#ifndef __HTAB_PRIVATE_H__
#define __HTAB_PRIVATE_H__

typedef struct htab_element {
    htab_pair_t element;
    struct htab_element *next;
}htab_item;

typedef struct htab {
    size_t size;
    size_t arr_size;
    htab_item *data[];
}htab_t;

#endif // __HTAB_PRIVATE_H__
