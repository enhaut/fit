// proj2.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 18. 4. 2021
#include <stdlib.h>
#include "proj2.h"

int get_number(bool *valid, signed int min, int max, char *raw_number)
{
    if (!valid)     // some of argument before is not valid also
        return 0;

    char *end;
    long numeric = strtol(raw_number, &end, 10);

    if (*end != '\0' || numeric <= min || numeric >= (long)max)
        *valid = false;

    return (int)numeric;
}

processes_t parse_arguments(int argc, char *args[])
{
    processes_t arguments;
    if (argc != 5)
        arguments.valid = false;
    else
    {
        bool valid = true;
        arguments.NE = get_number(&valid, ABOVE_0, NE_LIMIT, args[1]);
        arguments.NR = get_number(&valid, ABOVE_0, NR_LIMIT, args[2]);
        arguments.TE = get_number(&valid, ABOVE_EQUAL_0, TIMER_LIMIT, args[3]);
        arguments.TR = get_number(&valid, ABOVE_EQUAL_0, TIMER_LIMIT, args[4]);
        arguments.valid = valid;
    }
    if (!arguments.valid)
        ERROR_MESSAGE("Invalid arguments!\n");

    return arguments;
}

int main(int argc, char *args[])
{
    processes_t arguments = parse_arguments(argc, args);
    if (!arguments.valid)
        return EXIT_FAILURE;

    printf("%d, %d, %d, %d\n", arguments.NE, arguments.NR, arguments.TE, arguments.TR);
    return EXIT_SUCCESS;
}
