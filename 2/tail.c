// tail.c
// Řešení IJC-DU2, příklad a), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// ...popis příkladu - poznámky, omezení, atd

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXIMUM_LINE_LENGTH 1022
#define DEFAULT_LINES_TO_PRINT 10
#define ERROR_AND_RETURN(error_message, what_to_return) do{fprintf(stderr, error_message); return what_to_return;}while(0)

// Function parses -n argument also checks the count of arguments.
unsigned get_tail_start(int argc, char **args, bool *start_at)
{
    unsigned long starting_line;    // will be initialized in switch cases or function will be returned before using the variable

    switch (argc)
    {
        case 1:     // "tail <soubor"
            starting_line = DEFAULT_LINES_TO_PRINT;
            break;
        case 2:     // "tail soubor" / "tail -n"
            if (strncmp(args[1], "-n", 3) == 0)
                ERROR_AND_RETURN("Zadej číslo řádku!", 0);
            starting_line = DEFAULT_LINES_TO_PRINT;
            break;
        case 3:     // "tail -n 3 <soubor"
        case 4:     // "tail -n 3 soubor"
            if (strncmp(args[1], "-n", 3) != 0)
                ERROR_AND_RETURN("Nesprávne argumenty!", 0);

            if (args[2][0] == '+')
                *start_at = true;
            else if(args[2][0] == '-')
                ERROR_AND_RETURN("Číslo řádku musí být větší než 0!", 0);

            char *correctly_converted = NULL;
            starting_line = strtoul(args[2], &correctly_converted, 10);

            // TODO: check maximum num_row

            if (!correctly_converted || !starting_line)
                ERROR_AND_RETURN("Nesprávne číslo řádku!", 0);
            break;
        default:
            ERROR_AND_RETURN("Nesprávne argumenty!", 0);
    }

    return starting_line;
}

int main(int argc, char *args[])
{
    bool start_at = false;
    unsigned long staring_line = get_tail_start(argc, args, &start_at);
    if (!staring_line)  // 0 is not valid row number so it is used as "signal" value
        return 1;

    printf("\n%lu, %d", staring_line, start_at);
}