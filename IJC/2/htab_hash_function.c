// htab_hash_function.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// htab_hash_function() sa nemusí použiť ak je definovaný prepínač HASHTEST,
// vtedy sa použije htab_hash_function() zo súboru wordcount.c

#include <stdint.h>
#include "htab.h"

/**
 * @brief Hash function, it returns hash of provided string
 * @param str Pointer to the string to hash
 * @returns hash of provided string
 */
size_t htab_hash_function(const char *str) {
    uint32_t h = 0;     // musí mít 32 bitů
    const unsigned char *p;
    for(p = (const unsigned char*)str; *p != '\0'; p++)
        h = 65599 * h + *p;
    return h;
}
