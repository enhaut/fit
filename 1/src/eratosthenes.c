// eratosthenes.c
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include <math.h>
#include <string.h>
#include "eratosthenes.h"


int Eratosthenes(bitset_t bitset_array)
{
    bitset_setbit(bitset_array, 0, 1);
    bitset_setbit(bitset_array, 1, 1);

    bitset_index_t available_values = bitset_size(bitset_array);  // faster than using macro everywhere
    memset(&(bitset_array[1]), 0x55, available_values / BITS_PER_ULONG);

    bitset_index_t size = (bitset_index_t)sqrt(available_values);
    for (bitset_index_t i = 3; i < size; i += 2)
    {
        if (bitset_getbit(bitset_array, i))  // skip prime numbers
            continue;

        for (bitset_index_t n = i+i; n < available_values; n += i)
            bitset_setbit(bitset_array, n, 1);

    }

    return 0;
}
