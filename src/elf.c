// elf.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include "stdio.h"
#include <stdlib.h>
#include "elf.h"

int elf(shared_data_t *data, processes_t *arguments, int elfID)
{
    (void)data;
    (void)arguments;
    printf("A: Elf %d: started\n", elfID + 1);

    return EXIT_SUCCESS;
}
