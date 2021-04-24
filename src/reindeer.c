// reindeer.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "reindeer.h"
#include "proj2.h"

void vacation(shared_data_t *data, processes_t *arguments, int rdID)
{
    srand(getpid());
    int sleep_for = (rand() % (arguments->TR - (arguments->TR/2) + 1)) + arguments->TR/2;
    usleep(sleep_for*1000);
    correct_print(data, "A: RD %d: return home", rdID);
}

void wait_for_hitch(shared_data_t *data, int rdID)
{
    sem_wait(&(data->sems.reindeers));

    sem_wait(&(data->sems.mutex));
    data->all_reindeers_back -= 1;
    correct_print(data, "A: RD %d: get hitched", rdID);

    sem_post(&(data->sems.mutex));
}

int reindeer(shared_data_t *data, processes_t *arguments, int rdID)
{
    rdID++;     // reindeers are indexed from 1
    correct_print(data, "A: RD %d: rstarted", rdID);

    vacation(data, arguments, rdID);

    sem_wait(&(data->sems.mutex));
    data->all_reindeers_back += 1;
    if (data->all_reindeers_back == arguments->NR)
        sem_post(&(data->sems.santa));
    sem_post(&(data->sems.mutex));

    wait_for_hitch(data, rdID);

    return EXIT_SUCCESS;
}
