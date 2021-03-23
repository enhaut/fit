// bitset.h
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include "stdlib.h"
#include "error.h"

#ifndef BITSET_H
    #define BITSET_H

    typedef unsigned long *bitset_t;
    typedef unsigned long bitset_index_t;


    #define BITS_PER_ULONG (sizeof(unsigned long) * CHAR_BIT)
    #define ULONG_ARRAY_SIZE(velikost) (1 + (velikost / BITS_PER_ULONG) + (velikost % BITS_PER_ULONG ? 1 : 0))
    #define MAX_ARRAY_SIZE 4294967295   // 2^32


    #define bitset_create(jmeno_pole,velikost) static_assert(velikost > 0 && velikost < MAX_ARRAY_SIZE, "Velikost pole musí byt větší než 0!");unsigned long jmeno_pole[ULONG_ARRAY_SIZE(velikost)] = {velikost, 0};
    #define bitset_alloc(jmeno_pole,velikost) assert((velikost > 0 && velikost < MAX_ARRAY_SIZE) && "Nesprávna velikost pole!");bitset_t jmeno_pole = calloc(ULONG_ARRAY_SIZE(velikost), sizeof(unsigned long));if(!jmeno_pole){error_exit("bitset_alloc: Chyba alokace paměti\n");}jmeno_pole[0]=velikost;


    #ifndef USE_INLINE
        #define bitset_free(jmeno_pole) free(jmeno_pole)
        #define bitset_size(jmeno_pole) (jmeno_pole[0])
        #define bitset_setbit(jmeno_pole,index,vyraz) (index > bitset_size(jmeno_pole) ? (error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu",(unsigned long)index, bitset_size(jmeno_pole)), 0):(vyraz ? (jmeno_pole[1 + (index / BITS_PER_ULONG)] |= 1UL << index % BITS_PER_ULONG) : (jmeno_pole[1 + (index / BITS_PER_ULONG)] &= ~(1UL << (index % BITS_PER_ULONG)))))
        #define bitset_getbit(jmeno_pole,index) (index > bitset_size(jmeno_pole) ? (error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu",index, bitset_size(jmeno_pole)), 0) : (jmeno_pole[1 + (index / BITS_PER_ULONG)] & (1UL << (index) % BITS_PER_ULONG)) != 0)
        #else

        inline void bitset_free(bitset_t pole)
        {
            free(pole);
        }

        inline unsigned long bitset_size(bitset_t jmeno_pole)
        {
            return jmeno_pole[0];
        }

        inline void bitset_setbit(bitset_t jmeno_pole, bitset_index_t index, int vyraz)
        {
            if (index > bitset_size(jmeno_pole))
                error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu",(unsigned long)index, bitset_size(jmeno_pole));

            if (vyraz)
                jmeno_pole[1 + (index / BITS_PER_ULONG)] |= 1UL << index % BITS_PER_ULONG;
            else
                jmeno_pole[1 + (index / BITS_PER_ULONG)] &= ~(1UL << (index % BITS_PER_ULONG));
        }

        inline int bitset_getbit(bitset_t jmeno_pole, bitset_index_t index)
        {
            if (index > bitset_size(jmeno_pole))
                error_exit("bitset_getbit: Index %lu mimo rozsah 0..%lu",(unsigned long)index, bitset_size(jmeno_pole));

            return ((jmeno_pole[1 + (index / BITS_PER_ULONG)] & (1UL << (index) % BITS_PER_ULONG)) != 0);
        }

    #endif // USE_INLINE
#endif // BITSET_H
