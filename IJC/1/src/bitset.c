// bitset.c
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Tento subor slúži len na externú definíciu inline funkcii, pri -O0 by bez toho nefungovali.

#include "bitset.h"

#ifdef USE_INLINE
    extern void bitset_free(bitset_t pole);
    extern unsigned long bitset_size(bitset_t jmeno_pole);
    extern void bitset_setbit(bitset_t jmeno_pole, bitset_index_t index, int vyraz);
    extern int bitset_getbit(bitset_t jmeno_pole, bitset_index_t index);
#endif
