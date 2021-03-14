//
// Created by enhaut on 13. 3. 2021.
//

#include <time.h>
#include "bitset.h"
#include "eratosthenes.h"

int main(void)
{
    clock_t start = clock();

    bitset_alloc(bitset_array, 200000000);
    //printf("%lu", bitset_size(bitset_array));
    Eratosthenes(bitset_array);

    fprintf(stderr, "Time=%.3g\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    return 0;
}
