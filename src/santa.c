//
// Created by enhaut on 18. 4. 2021.
//

#include <stdlib.h>
#include "limits.h"
#include "santa.h"
#include "proj2.h"

void help(shared_data_t *data)
{
    correct_print(data, "Santa: helping elves");
    for (int i = 0; i < 3; i++)
        sem_post(&(data->sems.elves_in));
}

void sleep(shared_data_t *data)
{
    correct_print(data,"Santa: going to sleep");
}

void close_workshop(shared_data_t *data, processes_t *arguments)
{
    correct_print(data, "Santa: closing workshop");
    int to_hitch = data->all_reindeers_back;
    for (int i = 0; i < to_hitch; i++)
        sem_post(&(data->sems.reindeers));

    data->closed = true;
}

int santa(shared_data_t *data, processes_t *arguments)
{
    bool can_continue = true;
    while(can_continue)
    {
        sleep(data);
        sem_wait(&(data->sems.santa));      // waiting for wake up
        sem_wait(&(data->sems.mutex));      // waiting for mutex

        bool helping = false;
        if (data->all_reindeers_back == arguments->NR)
        {
            close_workshop(data, arguments);
            can_continue = false;

            /*********************************/
            /*RELEASING WAITING ELVES*/
            for (int i = 0 ; i < (arguments->NE - data->waiting_elves); i++)
            {
                sem_post(&(data->sems.elves_mutex));
                sem_post(&(data->sems.mutex));
            }
            for (int i = 0; i < 3; i++)
                sem_post(&data->sems.elves_in);
            /*********************************/

        } else if (data->waiting_elves == 3)
        {
            help(data);
            helping = true;
        }
        sem_post(&(data->sems.mutex));

        if (helping)
            for (int i = 0; i < 3; i++)
                sem_wait(&data->sems.waiting_santa);
    }

    for (int i = 0; i < arguments->NR; i++)
        sem_wait(&(data->sems.waiting_santa));

    correct_print(data, "Santa: Christmas started");

    fclose(data->log_file);

    return EXIT_SUCCESS;
}
