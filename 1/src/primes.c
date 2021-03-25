// primes.h
// Řešení IJC-DU1, příklad a), 6.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include <time.h>
#include "bitset.h"
#include "eratosthenes.h"

int main(void)
{
    clock_t start = clock();

    bitset_create(bitset_array, 200000000);
    Eratosthenes(bitset_array);

    unsigned long primes_to_print[10];

    int got_primes = 0;
    for (bitset_index_t i = bitset_size(bitset_array) - 1; i > 0  && got_primes < 10; i--)
    //                                                ^^ -1 because array is indexed from 0
    {
        if (bitset_getbit(bitset_array, i))
            continue;

        primes_to_print[got_primes] = i;
        got_primes++;
    }

    if (got_primes < 10)
        warning_msg("Zadaný rozsah neobsahuje dostatek (10) prvočísel!");
    else
        for (int i = 9; i >= 0; i--)
            printf("%ld\n", primes_to_print[i]);

    //bitset_free(bitset_array);    // free() at stack array is undefined

    fprintf(stderr, "Time=%.3g\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
