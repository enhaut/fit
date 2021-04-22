//
// Created by enhaut on 18. 4. 2021.
//

#include <stdlib.h>
#include <stdio.h>
#include "santa.h"
#include "proj2.h"

void sleep(shared_data_t *data)
{
    printf("A: Santa: going to sleep\n");
}

int santa(shared_data_t *data, processes_t *arguments)
{
    (void)arguments;
    sleep(data);

    return EXIT_SUCCESS;
}
