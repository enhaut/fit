// elf.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 22. 4. 2021

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "proj2.h"
#include "elf.h"

FILE *elf_log_file = NULL;

bool is_closed(shared_data_t *data, int elfID)
{
    if (!data->closed)
        return false;

    correct_print(data, "Elf %d: taking holidays", elfID);
    sem_post(&(data->sems.mutex));
    return true;
}

bool work(shared_data_t *data, processes_t *arguments, int elfID)
{
    srand(getpid());
    int sleep_for = (rand() % (arguments->TE + 1));
    usleep(sleep_for*1000);
    correct_print(data,"Elf %d: need help", elfID);
    return true;
}
/*
bool get_help(shared_data_t *data, int elfID)
{
    sem_wait(&(data->sems.getting_help));
    sem_wait(&(data->sems.mutex));
    if (is_closed(data, elfID))
        return false;
    correct_print(data, "Elf %d: get help", elfID);
    sem_post(&(data->sems.waiting_santa));
    sem_post(&(data->sems.mutex));
    return true;
}
*/

void elf_exit_handler(int signum)
{
    fclose(elf_log_file);
    exit(1);
}

int elf(shared_data_t *data, processes_t *arguments, int elfID)
{
    elf_log_file = data->log_file;
    signal(SIGUSR1, elf_exit_handler);

    elfID++;    // elf ids are indexed from 1
    correct_print(data, "Elf %d: started", elfID);

    while (true)
    {
        work(data, arguments, elfID);

        sem_wait(&data->sems.elves_mutex);
        sem_wait(&data->sems.mutex);
        if (data->closed)
        {
            sem_post(&data->sems.mutex);
            break;
        }

        data->waiting_elves += 1;
        if (data->waiting_elves == 3)
        {
            //correct_print(data, "pijng %d", data->waiting_elves);
            sem_post(&data->sems.santa);
        }else
            sem_post(&data->sems.elves_mutex);
        sem_post(&data->sems.mutex);


        sem_wait(&(data->sems.elves_in));

        sem_wait(&data->sems.mutex);
        if (data->closed)
        {
            sem_post(&data->sems.mutex);
            break;
        }
        correct_print(data, "Elf %d: get help", elfID);
        sem_post(&data->sems.waiting_santa);
        data->waiting_elves -= 1;
        if (data->waiting_elves == 0)
            sem_post(&(data->sems.elves_mutex));

        sem_post(&data->sems.mutex);
    }
    correct_print(data, "Elf %d: taking holidays", elfID);

    fclose(data->log_file);
    return EXIT_SUCCESS;
}
