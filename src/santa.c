//
// Created by enhaut on 18. 4. 2021.
//

#include <stdlib.h>
#include "santa.h"
#include "proj2.h"

void help(shared_data_t *data, bool silent)
{
    if (!silent)
        correct_print(data, "Santa: helping elves");

    int waiting = data->waiting_elves;
    for (int i = 0; i < waiting && i < 3; i++)   // help elves
        sem_post(&(data->sems.elves_in));

}

void sleep(shared_data_t *data)
{
    correct_print(data,"Santa: going to sleep");
}

void close_workshop(shared_data_t *data)
{
    correct_print(data, "Santa: closing workshop");
    int to_hitch = data->all_reindeers_back;
    for (int i = 0; i < to_hitch; i++)
        sem_post(&(data->sems.reindeers));

    data->closed = true;
    help(data, true);   // release all the waiting elves
}

bool wait_for_xmas(shared_data_t *data)
{
    bool started = false;

    sem_wait(&(data->sems.mutex));
    if (data->all_reindeers_back == 0)
        started = true;

    sem_post(&(data->sems.mutex));
    return started;
}

int santa(shared_data_t *data, processes_t *arguments)
{
    bool can_continue = true;
    while(can_continue)
    {
        sleep(data);
        sem_wait(&(data->sems.santa));      // waiting for wake up
        sem_wait(&(data->sems.mutex));      // waiting for mutex

        if (data->waiting_elves == 3)
            help(data, false);
        else if (data->all_reindeers_back == arguments->NR)
        {
            close_workshop(data);
            can_continue = false;
        }
        int remaining_elves = data->waiting_elves - 3;
        sem_post(&(data->sems.mutex));

        // TODO: remove active waiting
        bool all = true;    // waiting for all elves are out of santa's workshop
        while (all)
        {
            sem_wait(&(data->sems.mutex));
            if (!can_continue || (data->waiting_elves == remaining_elves && data->waiting_elves % 3 == 0))
                all = false;
            sem_post(&(data->sems.mutex));
        }
    }

    // TODO: remove active waiting
    while(!wait_for_xmas(data))
        continue;
    correct_print(data, "Santa: Christmas started");

    return EXIT_SUCCESS;
}
