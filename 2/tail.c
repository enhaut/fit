// tail.c
// Řešení IJC-DU2, příklad a), 27.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXIMUM_LINE_LENGTH 1023    // maximum # of characters in line WITHOUT trailing \0
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

void free_rows(char **rows, unsigned long rows_num)
{
    if (rows)
    {
        for (unsigned long row = 0; row < rows_num; row++)
        {
            if (rows[row])
                free(rows[row]);
            else
                break;
        }
        free(rows);
    }
}

char **allocate_rows_memory(unsigned long rows_num)
{
    bool failed = false;
    char **rows = calloc(rows_num, sizeof(char *));
    if (!rows)
        failed = true;

    for (unsigned long row_index = 0; row_index < rows_num && !failed; row_index++)
    {
        char *row = malloc((MAXIMUM_LINE_LENGTH + 1) * sizeof(char));
        if (!row)
            failed = true;
        rows[row_index] = row;  // assigning in case of failed malloc does not matter, it won't be used
    }

    if (failed)
    {
        free_rows(rows, rows_num);
        ERROR_AND_RETURN("Nepodařilo se alokovat paměť pro řádky!", NULL);
    }
    return rows;
}

FILE *get_input(int argc, char *args[])
{
    FILE *input = NULL;

    switch (argc)
    {
        case 1:
        case 3:
            input = stdin;
            break;
        case 2:
        case 4:
            input = fopen(args[argc-1], "r");   // filename is always last argument
            if (!input)
                ERROR_AND_RETURN("Nepodařilo se otevřít soubor!", NULL);
            break;
        default:
            break;   // do nothing - function get_tail_start() returns in this case 1 so program ends before calling this function
    }
    return input;
}

#define NEXT_ROW_INDEX(buffer_start, buffer_size) ((buffer_start + 1) % buffer_size)
void read_lines(char *lines[], unsigned long buffer_size, unsigned long *buffer_start, FILE *input)
{
    int character;
    int character_index = 0;
    bool longer_line = false;   // variable used for skipping characters at position > MAXIMUM_LINE_LENGTH

    while ((character = getc(input)) != EOF)
    {
        if (character == '\n')
        {
            lines[*buffer_start][character_index] = '\0';
            *buffer_start = NEXT_ROW_INDEX(*buffer_start, buffer_size);
            longer_line = false;
            character_index = 0;
            continue;
        }
        else if (longer_line)
            continue;

        if (character_index == MAXIMUM_LINE_LENGTH - 1)     // -1 because of character_index is indexed from 0
        {
            fprintf(stderr, "Řádek je delší, než je povoleno!");
            longer_line = true;
        }

        lines[*buffer_start][character_index] = character;

        character_index++;
    }
}

void print_lines(char **lines, unsigned long buffer_size, unsigned long buffer_start)
{
    for (unsigned long i = 0; i < buffer_size; i++)
    {
        printf("%s\n", lines[buffer_start]);
        buffer_start = NEXT_ROW_INDEX(buffer_start, buffer_size);
    }
}

void print_non_trailing_lines(unsigned long starting_line, FILE *input)
{
    char line[MAXIMUM_LINE_LENGTH];
    int character;
    int character_index = 0;
    unsigned long actual_line = 1;
    bool longer_line = false;
    bool reached_printing_line = (actual_line >= starting_line);

    while ((character = getc(input)) != EOF)
    {
        if (character == '\n')
        {
            line[character_index] = '\0';
            character_index = 0;
            longer_line = false;
            actual_line++;
            if (reached_printing_line)
                printf("%s\n", line);
            reached_printing_line = (actual_line >= starting_line);
            continue;
        }

        if (longer_line || !reached_printing_line)
            continue;

        if (character_index == MAXIMUM_LINE_LENGTH - 1)     // -1 because of character_index is indexed from 0
        {
            fprintf(stderr, "Řádek je delší, než je povoleno!");
            longer_line = true;
        }

        line[character_index] = character;
        character_index++;
    }
}

int main(int argc, char *args[])
{
    bool start_at = false;
    unsigned long line_num = get_tail_start(argc, args, &start_at);
    if (!line_num)  // 0 is not valid row number so it is used as "signal" value
        return 1;

    FILE *input = get_input(argc, args);
    if (!input)
        return 1;

    if (!start_at)
    {
        char **rows = allocate_rows_memory(line_num);
        unsigned long buffer_start = 0;
        read_lines(rows, line_num, &buffer_start, input);
        print_lines(rows, line_num, buffer_start);
        free_rows(rows, line_num);
    }else
        print_non_trailing_lines(line_num, input);


    fclose(input);
}
