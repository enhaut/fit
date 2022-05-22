// io.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "stdio.h"
#include <ctype.h>

/** @brief Function reads word by word from provided file up to max characters,
 * but it returns the real length of the word.
 * @param s Pointer to the array which stores the word
 * @param max Maximum number of characters to read
 * @param f pointer to the file
 * @returns Length of loaded word
*/
int read_word(char *s, int max, FILE *f)
{
    if (!f || !s || max <= 0)   // reading N chars; N<=0 does not make sense
        return EOF;

    int character;
    int i = 0;

    for (; i < max; i++)
    {
        character = getc(f);
        if (isspace(character))
            break;
        else if(i + 1 == max)
        {
            int temp = getc(f);
            fprintf(stderr, "Word is longer than allowed!");
            if (!isspace(temp))
                while ((temp = getc(f)) != EOF)     // jump to the next word
                {
                    i++;    // to return real length of word
                    if (isspace(temp))
                        break;
                }

            break;  // for (; i < max; i++)
        }else if(character == EOF)
        {
            s[i] = '\0';
            return EOF;
        }
        s[i] = (char)character;
    }   // for
    s[i] = '\0';
    return i;
}
