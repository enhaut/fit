// reindeer.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "reindeer.h"
#include "proj2.h"

shared_data_t *reindeer_shared_data = NULL;  // this global variable is used by reindeer_exit_handler() ONLY, it is difficult to pass arguments to handler by signal() call

void vacation(shared_data_t *data, processes_t *arguments, int rdID)
{
    srand(getpid());
    int sleep_for = (rand() % (arguments->TR - (arguments->TR/2) + 1)) + arguments->TR/2;
    usleep(sleep_for*1000);
    correct_print(data, "RD %d: return home", rdID);
}

void wait_for_hitch(shared_data_t *data, int rdID)
{
    sem_wait(&(data->sems.reindeers));

    sem_wait(&(data->sems.mutex));
    data->reindeers--;
    correct_print(data, "RD %d: get hitched", rdID);
    sem_post(&(data->sems.mutex));
    sem_post(&(data->sems.waiting_santa));
}

void reindeer_exit_handler(int signum)
{
    (void)signum;  // disable unused warnings
    fclose(reindeer_shared_data->log_file);
    free(reindeer_shared_data->child_pids);
    exit(0);
}

int reindeer(shared_data_t *data, processes_t *arguments, int rdID)
{
    reindeer_shared_data = data;
    signal(SIGTERM, reindeer_exit_handler);

    rdID++;     // reindeers are indexed from 1
    correct_print(data, "RD %d: rstarted", rdID);

    vacation(data, arguments, rdID);

    sem_wait(&(data->sems.mutex));
    data->reindeers++;
    if (data->reindeers == arguments->NR)
        sem_post(&(data->sems.santa));
    sem_post(&(data->sems.mutex));

    wait_for_hitch(data, rdID);

    reindeer_exit_handler(0);
    return EXIT_SUCCESS;
}
