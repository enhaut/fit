//
// Created by enhaut on 17. 11. 2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define print_error(...) fprintf(stderr, __VA_ARGS__)
#define MINIMAL_ARGUMENTS_COUNT 3


bool provided_minimal_amount_of_arguments(int arg_count)
{
    if (arg_count >= MINIMAL_ARGUMENTS_COUNT)
        return false;
    print_error("Invalid arguments!");
    return true;
}

int main(int arg_count, char *arguments[])
{
    if (provided_minimal_amount_of_arguments(arg_count))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}