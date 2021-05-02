//
// Created by enhaut on 18. 4. 2021.
//

#include <stdlib.h>
#include <signal.h>
#include "santa.h"
#include "proj2.h"

shared_data_t *santa_shared_data = NULL;  // this global variable is used by santa_exit_handler() ONLY, it is difficult to pass arguments to handler by signal() call

void help(shared_data_t *data, bool *helping)
{
    correct_print(data, "Santa: helping elves");
    for (int i = 0; i < 3; i++)
        sem_post(&(data->sems.elves_in));
    *helping = true;
}

void sleep(shared_data_t *data)
{
    correct_print(data,"Santa: going to sleep");
}

void close_workshop(shared_data_t *data, processes_t *arguments, bool *can_continue)
{
    correct_print(data, "Santa: closing workshop");
    int to_hitch = data->reindeers;
    for (int i = 0; i < to_hitch; i++)
        sem_post(&(data->sems.reindeers));

    data->closed = true;
    *can_continue = false;

    /*RELEASING WAITING ELVES*/
    for (int i = 0 ; i < (arguments->NE - data->waiting_elves); i++)
    {
        sem_post(&(data->sems.elves_mutex));
        sem_post(&(data->sems.mutex));
    }
    for (int i = 0; i < 3; i++)
        sem_post(&data->sems.elves_in);

}

void santa_exit_handler(int signum)
{
    (void)signum;  // disable unused warnings
    fclose(santa_shared_data->log_file);
    free(santa_shared_data->child_pids);
    exit(0);
}

void wait_for_childs_to_process_help(shared_data_t *data, int wait_for)
{
    for (int i = 0; i < wait_for; i++)
        sem_wait(&data->sems.waiting_santa);
}

int santa(shared_data_t *data, processes_t *arguments)
{
    santa_shared_data = data;
    signal(SIGTERM, santa_exit_handler);

    bool can_continue = true;
    while(can_continue)
    {
        sleep(data);
        sem_wait(&(data->sems.santa));      // waiting for wake up
        sem_wait(&(data->sems.mutex));      // waiting for mutex

        bool helping = false;
        if (data->reindeers == arguments->NR)
            close_workshop(data, arguments, &can_continue);
        else if (data->waiting_elves == WAKING_UP_ELVES_NUMBER)
            help(data, &helping);

        sem_post(&(data->sems.mutex));

        if (helping)
            wait_for_childs_to_process_help(data, WAKING_UP_ELVES_NUMBER);  // wait for elves to "receive" help from santa
    }

    wait_for_childs_to_process_help(data, arguments->NR);  // wait for all reindeers to get hitched

    correct_print(data, "Santa: Christmas started");

    santa_exit_handler(0);
    return EXIT_SUCCESS;
}
