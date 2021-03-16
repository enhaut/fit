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
    bitset_setbit(pole, 0, 1);  // number "0" is not even
    bitset_setbit(pole, 1, 1);  // number "1" also

    bitset_index_t available_values = bitset_size(pole);  // faster than using macro everywhere (~20ms on my machine at average of 200tries)
    memset(&(pole[1]), 0x55, available_values / 8 + 1); // +1 because of not using the first element in array

    bitset_index_t size = (bitset_index_t)sqrt(available_values);
    for (bitset_index_t i = 2; i < size; i++)
    {
        if (bitset_getbit(pole, i))  // skip prime numbers
            continue;

        for (bitset_index_t n = i+i; n < available_values; n += i)
            bitset_setbit(pole, n, 1);
    }
}
