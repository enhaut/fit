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
#include "santa.h"

FILE *elf_log_file = NULL;

bool is_closed(shared_data_t *data)
{
    if (!data->closed)
        return false;

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

void process_santas_help(shared_data_t *data, int elfID)
{
    correct_print(data, "Elf %d: get help", elfID);
    sem_post(&data->sems.waiting_santa);
    data->waiting_elves--;
    if (data->waiting_elves == 0)
        sem_post(&(data->sems.elves_mutex));
}

void elf_exit_handler(int signum)
{
    (void)signum;  // disable unused warnings
    fclose(elf_log_file);
    exit(0);
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
        if (is_closed(data))
            break;

        data->waiting_elves++;
        if (data->waiting_elves == WAKING_UP_ELVES_NUMBER)
            sem_post(&data->sems.santa);
        else
            sem_post(&data->sems.elves_mutex);  // let one more elf waits to get help
        sem_post(&data->sems.mutex);


        sem_wait(&(data->sems.elves_in));  // wait to get help from Santa

        sem_wait(&data->sems.mutex);
        if (is_closed(data))
            break;
        process_santas_help(data, elfID);

        sem_post(&data->sems.mutex);
    }
    correct_print(data, "Elf %d: taking holidays", elfID);

    return EXIT_SUCCESS;
}
