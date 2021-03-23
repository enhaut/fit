// steg-decode.c
// Řešení IJC-DU1, příklad b), 11.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#include "ppm.h"
#include "bitset.h"
#include "eratosthenes.h"
#include <stdio.h>


char *add_character_to_message(char *message, char character, unsigned index)
{
    unsigned message_size = index ? index : 100; // first message size has to be != 0
    if (!message)
    {
        message = malloc(message_size + 1);
    }
    else if (index == message_size && character != 0)
    {
        message_size *= 2;
        message = realloc(message, message_size + 1);
    }
    if (!message)
    {
        warning_msg("Nepodařilo se alokovat místo pro zprávu!");
        return NULL;
    }

    message[index] = character;

    return message;
}

void read_encoded_message(struct ppm *loaded_image, bitset_t primes)
{
    char message_character = 0;
    unsigned character_index = 0;
    char *message = NULL;
    int message_correct_ending = 0;

    for (size_t i = 23, char_index = 0; i < bitset_size(primes); i++)
    {
        if (bitset_getbit(primes, i))       // skipping primes
            continue;

        if (loaded_image->data[i] % 2)      // LSb determines that number is odd/even
            message_character |= 1UL << char_index;

        char_index++;

        if (char_index == CHAR_BIT)
        {
            message = add_character_to_message(message, message_character, character_index);

            if (message_character == '\0')
            {
                message_correct_ending = 1;
                break;
            }

            character_index++;
            char_index = 0;
            message_character = 0;
        }
    }
    if (!message_correct_ending)
    {
        bitset_free(primes);
        free(message);
        ppm_free(loaded_image);
        error_exit("Zpráva není korektně zakončená!");
    }

    printf("%s\n", message);
    free(message);
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