// proj2.c
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 18. 4. 2021
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#include "proj2.h"
#include "santa.h"
#include "elf.h"
#include "reindeer.h"

int get_number(bool *valid, signed int min, int max, char *raw_number)
{
    if (!valid)     // some of argument before is not valid also
        return 0;

    char *end;
    long numeric = strtol(raw_number, &end, 10);

    if (*end != '\0' || numeric <= min || numeric >= (long)max)
        *valid = false;

    return (int)numeric;
}

processes_t parse_arguments(int argc, char *args[])
{
    processes_t arguments;
    if (argc != 5)
        arguments.valid = false;
    else
    {
        bool valid = true;
        arguments.NE = get_number(&valid, ABOVE_0, NE_LIMIT, args[1]);
        arguments.NR = get_number(&valid, ABOVE_0, NR_LIMIT, args[2]);
        arguments.TE = get_number(&valid, ABOVE_EQUAL_0, TIMER_LIMIT, args[3]);
        arguments.TR = get_number(&valid, ABOVE_EQUAL_0, TIMER_LIMIT, args[4]);
        arguments.valid = valid;
    }
    if (!arguments.valid)
        ERROR_EXIT("Invalid arguments!\n", arguments);

    return arguments;
}



int initialize_semaphores(shared_data_t *data)
{
    int failed = sem_init(&(data->sems.reindeers), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(data->sems.elves_in), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize elves semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(data->sems.mutex), 1, 1);
    if (failed)
        ERROR_EXIT("Could not initialize mutex semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(data->sems.santa), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(data->sems.print), 1, 1);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

   return EXIT_SUCCESS;
}

void destroy_semaphores(shared_data_t *data)
{
    sem_destroy(&(data->sems.reindeers));
    sem_destroy(&(data->sems.elves_in));
    sem_destroy(&(data->sems.mutex));
    sem_destroy(&(data->sems.santa));
    sem_destroy(&(data->sems.print));
}

void create_forks(shared_data_t *shared_data, processes_t *arguments)
{
    int santa_fork = fork();
    if (santa_fork)
        exit(santa(shared_data, arguments));

    for (int i = 0; i < arguments->NE; i++)
        if (fork())
            exit(elf(shared_data, arguments, i));

    for (int i = 0; i < arguments->NR; i++)
        if(fork())
            exit(reindeer(shared_data, arguments, i));

}


void correct_print(shared_data_t *data, const char *fmt, ...)
{
    sem_wait(&(data->sems.print));

    va_list args;
    va_start(args, fmt);

    vprintf(fmt, args);
    printf("\n");
    fflush(stdout);

    va_end(args);

    sem_post(&(data->sems.print));
}


int main(int argc, char *args[])
{
    processes_t arguments = parse_arguments(argc, args);
    if (!arguments.valid)
        return EXIT_FAILURE;

    int shared_mem_id = shmget(895664986, sizeof(shared_data_t), 0666 | IPC_CREAT);
    if (shared_mem_id < 0)
        ERROR_EXIT("Could not allocate shared memory!!!!\n", EXIT_FAILURE);

    shared_data_t *shared_data = shmat(shared_mem_id, NULL, 0);
    if (shared_data == (shared_data_t *) -1)
        ERROR_EXIT("Could not allocate shared memory!\n", EXIT_FAILURE);

    shared_data->shm_key = shared_mem_id;
    shared_data->waiting_elves = 0;
    shared_data->all_reindeers_back = 0;
    shared_data->closed = false;

    int initialized = initialize_semaphores(shared_data);
    if (initialized)
        return EXIT_FAILURE;


    create_forks(shared_data, &arguments);
    while(wait(NULL) > 0);  // waiting for child processes

    destroy_semaphores(shared_data);
    if(shmdt(shared_data))
        return EXIT_FAILURE;
    shmctl(shared_mem_id, IPC_RMID, NULL);

    return EXIT_SUCCESS;
}
