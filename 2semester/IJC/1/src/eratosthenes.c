// eratosthenes.c
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Modul s funkciou pre výpočet prvočísel pomocou metody Eratosthenesovho sita

#include <math.h>
#include <string.h>
#include "eratosthenes.h"

// Function will set 1 to index of non prime numbers in bit array.
// It uses Eratosthenes method.
void Eratosthenes(bitset_t pole)
{
    bitset_index_t available_values = bitset_size(pole);  // faster than using macro everywhere (~20ms on my machine at average of 200tries)
    size_t array_elements = (available_values / CHAR_BIT);
    memset(&(pole[1]), 0x55, array_elements);

    bitset_setbit(pole, 2, 0);  // 2 is prime
    bitset_setbit(pole, 1, 1);  // 1 is not prime

    // The last array member may not have set numbers dividable by 2 as non primes
    for (bitset_index_t i = array_elements * CHAR_BIT; i < available_values; i++)   // divided value * divider returns # of numbers, that has been marked by memset
    {
        if (i % 2 == 0)
            bitset_setbit(pole, i, 1);  // setting numbers dividable by 2 of last array element
    }

    bitset_index_t size = (bitset_index_t)sqrt(available_values) + 1;   // +1 because of round error
    for (bitset_index_t i = 3; i < size; i += 2)
    {
        if (bitset_getbit(pole, i))  // skip non prime numbers
            continue;

        for (bitset_index_t n = i*i; n < available_values; n += i*2)   // setting multiples of the number, because it's dividable by that number
            bitset_setbit(pole, n, 1);
    }
}
