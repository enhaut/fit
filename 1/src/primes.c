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

    bitset_alloc(bitset_array, 200000000);
    Eratosthenes(bitset_array);

    int got_primes = 0;
    for (bitset_index_t i = bitset_size(bitset_array); i > 0  && got_primes < 10; i--)
    {
        if (bitset_getbit(bitset_array, i))
            continue;

        printf("%ld\n", i);
        got_primes++;
    }

    bitset_free(bitset_array);

    fprintf(stderr, "Time=%.3g\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
