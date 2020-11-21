//
// Created by enhaut on 17. 11. 2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define print_error(...) fprintf(stderr, __VA_ARGS__ "\n")
#define MINIMAL_ARGUMENTS_COUNT 3

typedef unsigned long long table_index;     // rows and columns have no limit, so I am using ull

bool string_compare(char *first, char *second)
{
    return strcmp(first, second) == 0;
}

bool provided_minimal_amount_of_arguments(int arg_count)
{
    if (arg_count >= MINIMAL_ARGUMENTS_COUNT)
        return false;
    print_error("Invalid arguments!");
    return true;
}

// Returns -1 for invalid delimiter -> exit, 0 for no delimiter defined, 1 for defined delimiter
int defined_delimiter(int arg_count, char *arguments[])
{
    bool is_set_d_argument = string_compare(arguments[1], "-d");

    if (arg_count == 4 || (arg_count == 5 &&
                          (((is_set_d_argument && strlen(arguments[2]) == 0) || // delimiter value is empty
                            !is_set_d_argument) ||                                 // 2. argument is not -d
                            (strchr(arguments[2], '"') || strchr(arguments[2], '\\')   // forbidden chars in delimiter
                            ))))
    {
        print_error("Invalid delimiter!");
        return -1;
    }else if (arg_count == 5 && is_set_d_argument)
        return 1;

    return 0;
}

int get_table_filename_index(int arg_count)
{
    return arg_count - 1;   // filename is always last argument
}

FILE *file_loader(char *filename, char *mode)
{
    FILE *opened_file = fopen(filename, mode);
    if (!opened_file) {
        print_error("Could not open file!");
        return NULL;
    }
    return opened_file;
}

table_index get_table_file_rows(FILE *table_file)
{
    table_index rows = 0;
    int loaded_character;
    int character_before;

    while ((loaded_character = getc(table_file)) != EOF)
    {
        if (loaded_character == '\n' &&
            loaded_character != character_before)   // prevent counting empty lines
            rows++;
        character_before = loaded_character;
    }
    return rows;
}

int main(int arg_count, char *arguments[])
{
    if (provided_minimal_amount_of_arguments(arg_count))
        return EXIT_FAILURE;

    int is_defined_delimiter = defined_delimiter(arg_count, arguments);
    if (is_defined_delimiter == -1)
        return EXIT_FAILURE;

    char default_delimiter[] = " ";
    char *delimiter = default_delimiter;

    if (is_defined_delimiter)
        delimiter = arguments[2];


    FILE *table_file = file_loader(arguments[get_table_filename_index(arg_count)], "r");
    if (!table_file)
        return EXIT_FAILURE;

    table_index rows = get_table_file_rows(table_file);

    printf("%llu", rows);
    printf(".%s.", delimiter);
    return EXIT_SUCCESS;
}