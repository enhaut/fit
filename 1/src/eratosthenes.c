//
// Created by enhaut on 13. 3. 2021.
//

#include <math.h>
#include "eratosthenes.h"


int Eratosthenes(bitset_t bitset_array)
{
    bitset_setbit(bitset_array, 0, 1);
    bitset_setbit(bitset_array, 1, 1);


    unsigned long array_size = bitset_size(bitset_array);  // in average, this is faster
    for (bitset_index_t i = 2; i < array_size; i += 2)
        bitset_setbit(bitset_array, i, 1);

    bitset_index_t size = sqrt(bitset_size(bitset_array));
    for (bitset_index_t i = 2; i < size; i++)
    {
        if (bitset_getbit(bitset_array, i))  // skip prime numbers
            continue;

        for (bitset_index_t n = i+i; n < array_size; n += i)
            bitset_setbit(bitset_array, n, 1);

    }

    return 0;
}
