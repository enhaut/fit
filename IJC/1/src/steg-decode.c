// steg-decode.c
// Řešení IJC-DU1, příklad b), 11.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Modul slúžiaci pre čítanie "zašifrovaných" správ.

#include "ppm.h"
#include "bitset.h"
#include "eratosthenes.h"
#include <stdio.h>

// Function will add character to message array, in case there is no space left it will resize the array.
// In case, the first character is being added message==NULL, so function will allocate space also.
char *add_character_to_message(char *message, char character, unsigned index)
{
    unsigned message_size = index ? index : 100; // first message size has to be != 0

    if (!message)   // message array is not allocated yet
        message = malloc(message_size + 1);
    else if (index == message_size && character != 0)   // there is no reason to "resize" array if ending \0 can fit
    {
        char *resized_message = realloc(message, message_size * 2 + 1);
        if (!resized_message)
            free(message);
        message = resized_message;
    }

    if (message)
        message[index] = character;

    return message;
}

#define DEALLOC_MEMORY_BEFORE_EXIT(primes, message, image) do{bitset_free(primes);if(message){free(message);}ppm_free(image);}while(0)
// Function reads encoded message from loaded image data.
void read_encoded_message(struct ppm *loaded_image, bitset_t primes)
{
    char message_character = 0;     // character that is being read from image data
    unsigned character_index = 0;   // index of character in message
    int message_correct_ending = 0; // check if message has been correctly ended
    char *message = NULL;

    for (size_t i = 23, char_index = 0; i < bitset_size(primes); i++)
    {
        if (bitset_getbit(primes, i))       // skipping non primes
            continue;

        if (loaded_image->data[i] % 2)      // LSb determines that number is odd/even
            message_character |= 1UL << char_index;

        char_index++;

        if (char_index == CHAR_BIT) // character has been completely read
        {
            message = add_character_to_message(message, message_character, character_index);
            if (!message)
            {
                DEALLOC_MEMORY_BEFORE_EXIT(primes, message, loaded_image);
                error_exit("Nepodařilo se alokovat místo pro zprávu!");
            }

            if (message_character == '\0')
            {
                message_correct_ending = 1;
                break;
            }

            character_index++;
            char_index = 0;
            message_character = 0;
        } // if (char_index == CHAR_BIT)
    } // for

    if (!message_correct_ending)
    {
        DEALLOC_MEMORY_BEFORE_EXIT(primes, message, loaded_image);
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

    // message characters are at bytes not bites so i need up to (image_bites)/no_of_bites_in_byte prime numbers
    // +1 to fix possible round error
    size_t image_size = (loaded_image->xsize * loaded_image->ysize * 3) / CHAR_BIT + 1;
    bitset_alloc(primes, image_size);
    Eratosthenes(primes);

    read_encoded_message(loaded_image, primes);

    DEALLOC_MEMORY_BEFORE_EXIT(primes, NULL, loaded_image);
    return 0;
}
