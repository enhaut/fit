#include <stdlib.h>
#include "htab.h"

// následující řádky zabrání násobnému vložení:
#ifndef __HTAB_PRIVATE_H__
#define __HTAB_PRIVATE_H__

#define PRINT_ERROR(message) do{fprintf(stderr, message);}while(0)

typedef struct htab_element {
    htab_pair_t element;
    struct htab_element *next;
}htab_element_t;

typedef struct htab {
    size_t size;
    size_t arr_size;
    htab_element_t *data[];
}htab_t;

#endif // __HTAB_PRIVATE_H__
