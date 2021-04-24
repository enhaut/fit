// elf.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include <stdlib.h>
#include <unistd.h>
#include "proj2.h"
#include "elf.h"


bool is_closed(shared_data_t *data, int elfID)
{
    if (!data->closed)
        return false;

    correct_print(data, "A: Elf %d: taking holidays", elfID);
    sem_post(&(data->sems.mutex));
    return true;
}

bool work(processes_t *arguments, int elfID, shared_data_t *data)
{
    srand(getpid());
    int sleep_for = (rand() % (arguments->TE + 1));
    usleep(sleep_for*1000);
    correct_print(data,"A: Elf %d: need help", elfID);
    return true;
}

bool get_help(shared_data_t *data, int elfID)
{
    sem_wait(&(data->sems.elves_in));   // santa will release him
    sem_wait(&(data->sems.mutex));   // santa will release him
    if (is_closed(data, elfID))
        return false;

    data->waiting_elves -= 1;
    correct_print(data, "A: Elf %d: get help", elfID);
    sem_post(&(data->sems.mutex));   // santa will release him
    return true;
}

int elf(shared_data_t *data, processes_t *arguments, int elfID)
{
    elfID++;    // elf ids are indexed from 1
    correct_print(data, "A: Elf %d: started", elfID);

    while (true)
    {
        work(arguments, elfID, data);

        sem_wait(&(data->sems.mutex));
        if (is_closed(data, elfID))
            break;

        data->waiting_elves += 1;
        if (data->waiting_elves == 3)
            sem_post(&(data->sems.santa));
        sem_post(&(data->sems.mutex));

        if (!get_help(data, elfID))
            break;
    }
    return EXIT_SUCCESS;
}
