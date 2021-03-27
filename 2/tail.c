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

unsigned get_lines_count(int argc, char *args[], bool *start_at)
{
    unsigned long starting_line = 0;
    bool is_n_parameter = false;
    if (argc >= 3 && argc <= 4 && (is_n_parameter = strncpy(args[1], "-n", 3)))
    {
        if (args[2][0] == '+')
            *start_at = true;
        else if(args[2][0] == '-')
            ERROR_AND_RETURN("Číslo řádku musí být větší než 0!", 0);

        char *correctly_converted = NULL;
        starting_line = strtoul(args[2], &correctly_converted, 10);

        if (!correctly_converted || !starting_line)
            ERROR_AND_RETURN("Nesprávne číslo řádku!", 0);

    }else if(argc == 2 && is_n_parameter)
        ERROR_AND_RETURN("Nesprávne argumenty!", 0);

    return starting_line ? starting_line : DEFAULT_LINES_TO_PRINT;
}

int main(int argc, char *args[])
{
    bool start_at = false;
    unsigned long staring_line = get_lines_count(argc, args, &start_at);
    //if (!staring_line)
    //    return 1;

    printf("\n%lu, %d", staring_line, start_at);
}