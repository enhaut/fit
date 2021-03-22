// steg-decode.c
// Řešení IJC-DU1, příklad b), 11.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include "steg-decode.h"
#include "ppm.h"
#include "bitset.h"
#include "eratosthenes.h"
#include <stdio.h>


void read_encoded_message(struct ppm *loaded_image, bitset_t primes)
{
    char message_character = 0;

    for (size_t i = 23, char_index = 0; i < bitset_size(primes); i++)
    {
        if (bitset_getbit(primes, i))       // skipping primes
            continue;

        if (loaded_image->data[i] % 2)      // LSb determines that number is odd/even
            message_character |= 1UL << char_index;

        char_index++;
        if (char_index == CHAR_BIT && !message_character)
        {
            printf("\n");
            break;
        }
        else if (char_index == CHAR_BIT)
        {
            printf("%c", message_character);
            char_index = 0;
            message_character = 0;
        }
        else if (i + 1 == bitset_size(primes))
        {
            bitset_free(primes);
            ppm_free(loaded_image);
            error_exit("Zpráva není korektně zakončená!");
        }
    }
}

int main(int argc, char *args[])
{
    if (argc < 2)
        error_exit("Je potřeba zadat jméno souboru!");

    struct ppm *loaded_image = ppm_read(args[1]);
    if (!loaded_image)
        return 1;

    size_t image_size = loaded_image->xsize * loaded_image->ysize * 3;
    bitset_alloc(primes, image_size);
    Eratosthenes(primes);

    read_encoded_message(loaded_image, primes);

    bitset_free(primes);
    ppm_free(loaded_image);
    return 0;
}