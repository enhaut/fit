// eratosthenes.c
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include <math.h>
#include <string.h>
#include "eratosthenes.h"


void Eratosthenes(bitset_t pole)
{
    bitset_index_t available_values = bitset_size(pole);  // faster than using macro everywhere (~20ms on my machine at average of 200tries)
    size_t array_elements = (available_values / CHAR_BIT);
    memset(&(pole[1]), 0x55, array_elements);

    for (bitset_index_t i = array_elements * CHAR_BIT; i < available_values; i++)
        bitset_setbit(pole, i, 1);  // setting numbers dividable by 2 of last array element

    bitset_index_t size = (bitset_index_t)sqrt(available_values);
    for (bitset_index_t i = 3; i < size; i += 2)
    {
        if (bitset_getbit(pole, i))  // skip prime numbers
            continue;

        for (bitset_index_t n = i*i; n <= available_values; n += i*2)
            bitset_setbit(pole, n, 1);
    }
}
