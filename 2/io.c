// io.c
// Řešení IJC-DU2, příklad b), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include "stdio.h"
#include <ctype.h>

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
                while ((temp = getc(f)) != EOF) {if (isspace(temp)) {break;}}   // jump to the next word

            break;
        }else if(character == EOF)
        {
            s[i] = '\0';
            return EOF;
        }
        s[i] = (char)character;
    }
    s[i] = '\0';
    return i;
}
