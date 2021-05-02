//
// Created by enhaut on 18. 4. 2021.
//

#include <stdlib.h>
#include <signal.h>
#include "santa.h"
#include "proj2.h"

FILE *santa_log_file = NULL;

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
    int to_hitch = data->reindeers;
    for (int i = 0; i < to_hitch; i++)
        sem_post(&(data->sems.reindeers));

    data->closed = true;
}

void santa_exit_handler(int signum)
{
    (void)signum;  // disable unused warnings
    fclose(santa_log_file);
    exit(0);
}

int santa(shared_data_t *data, processes_t *arguments)
{
    santa_log_file = data->log_file;
    signal(SIGUSR1, santa_exit_handler);

    bool can_continue = true;
    while(can_continue)
    {
        sleep(data);
        sem_wait(&(data->sems.santa));      // waiting for wake up
        sem_wait(&(data->sems.mutex));      // waiting for mutex

        bool helping = false;
        if (data->reindeers == arguments->NR)
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
