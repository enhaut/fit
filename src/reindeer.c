// reindeer.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include <stdlib.h>
#include <stdio.h>
#include "reindeer.h"

int reindeer(shared_data_t *data, processes_t *arguments, int rdID)
{
    (void)data;
    (void)arguments;
    printf("A: RD %d: rstarted\n", rdID);


    return EXIT_SUCCESS;
}
