// bitset.h
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "stdlib.h"

typedef unsigned long bitset_t[];
typedef unsigned long bitset_index_t;

#define BITS_PER_ULONG (sizeof(unsigned long) * CHAR_BIT)
#define ULONG_ARRAY_SIZE(velikost) (1 + (velikost / BITS_PER_ULONG) + (velikost % BITS_PER_ULONG ? 1 : 0))
#define MAX_ARRAY_SIZE (2^(sizeof(void *) * CHAR_BIT) +100000000000000000) // TODO: dorobit velkost

#define bitset_create(jmeno_pole,velikost) static_assert(velikost > 0, "Velikost pole musí byt větší než 0!");unsigned long jmeno_pole[ULONG_ARRAY_SIZE(velikost)] = {velikost, 0};
#define bitset_alloc(jmeno_pole,velikost) static_assert(velikost > 0 && velikost < MAX_ARRAY_SIZE,"Nesprávna délka pole");unsigned long *jmeno_pole = calloc(ULONG_ARRAY_SIZE(velikost), sizeof(unsigned long));if(!jmeno_pole){fprintf(stderr, "bitset_alloc: Chyba alokace paměti\n");return 1;}
#define bitset_free(jmeno_pole) free(jmeno_pole)
#define bitset_size(jmeno_pole) (jmeno_pole[0])
