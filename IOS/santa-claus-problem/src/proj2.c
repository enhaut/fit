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

shared_data_t *shared_data = NULL;  // using global variable because of correct exit after SIGQUIT is received

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

int initialize_semaphores()
{
    int failed = sem_init(&(shared_data->sems.reindeers), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.elves_in), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize elves semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.elves_mutex), 1, 1);
    if (failed)
        ERROR_EXIT("Could not initialize elves semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.mutex), 1, 1);
    if (failed)
        ERROR_EXIT("Could not initialize mutex semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.santa), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.waiting_santa), 1, 0);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

    failed = sem_init(&(shared_data->sems.print), 1, 1);
    if (failed)
        ERROR_EXIT("Could not initialize reindeers semaphore!\n", EXIT_FAILURE);

   return EXIT_SUCCESS;
}

void destroy_semaphores()
{
    sem_destroy(&(shared_data->sems.reindeers));
    sem_destroy(&(shared_data->sems.elves_in));
    sem_destroy(&(shared_data->sems.elves_mutex));
    sem_destroy(&(shared_data->sems.mutex));
    sem_destroy(&(shared_data->sems.santa));
    sem_destroy(&(shared_data->sems.waiting_santa));
    sem_destroy(&(shared_data->sems.print));
}

void kill_child_processes()
{
    for (int i = 1; i < shared_data->child_pids[0]; i++) // starting at 1 because 0. element is sizeof array
        if (shared_data->child_pids[i])    // killing just successfully forked processes
            kill(shared_data->child_pids[i], SIGTERM);
}

void initialize_pids_array(int size)
{
    int *pids = calloc(size + 1, sizeof(int));  // +1 because first element is size of array
    if (!pids)
        return;
    pids[0] = size;

    shared_data->child_pids = pids;

}

int create_forks(processes_t *arguments)
{
    int processes = 1 + arguments->NE + arguments->NR;
    initialize_pids_array(processes);
    if (!shared_data->child_pids)
        ERROR_EXIT("Could not allocate memory for PIDs array", EXIT_FAILURE);

    bool failed = false;

    CREATE_FORK(&failed, 0, shared_data->child_pids, santa, shared_data, arguments);

    for (int i = 0; i < arguments->NE && !failed; i++)
        CREATE_FORK(&failed, 1+i, shared_data->child_pids, elf, shared_data, arguments, i);


    for (int i = 0; i < arguments->NR && !failed; i++)
        CREATE_FORK(&failed, 1+arguments->NE + i, shared_data->child_pids, reindeer, shared_data, arguments, i);
    if (failed)
    {
        kill_child_processes();
        ERROR_EXIT("Could not start processes!\n", EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}


void correct_print(shared_data_t *data, const char *fmt, ...)
{
    sem_wait(&(data->sems.print));

    va_list args;
    va_start(args, fmt);

    fprintf(data->log_file, "%d: ", ++(data->message_counter));
    vfprintf(data->log_file, fmt, args);
    fprintf(data->log_file, "\n");
    fflush(data->log_file);

    va_end(args);

    sem_post(&(data->sems.print));
}

void delete_pid(int *pids, int pid)
{
    for (int i = 1; i < pids[0]; i++)
        if (pids[i] == pid)
            pids[i] = 0;
}

FILE *initialize_log_file()
{
    FILE *file = fopen("proj2.out", "w");
    if (!file)
    {
        fprintf(stderr, "Could not open log file!");
        return NULL;
    }
    return file;
}

void wait_for_child_processes()
{
    while(wait(NULL) > 0);
}

int initialize_shared_memory()
{
    shared_data->waiting_elves = 0;
    shared_data->reindeers = 0;
    shared_data->message_counter = 0;
    shared_data->closed = false;
    shared_data->child_pids = NULL;
    shared_data->log_file = initialize_log_file();
    if (!shared_data->log_file)
        ERROR_EXIT("Could not open log file!\n", EXIT_FAILURE);

    return EXIT_SUCCESS;
}

void free_shared_memory(shared_data_t *data)
{
    shmctl(data->shm_id, IPC_RMID, NULL);
    shmdt(data);
}

void free_initialized(int signum)
{
    (void)signum;
    kill_child_processes();
    fclose(shared_data->log_file);
    free(shared_data->child_pids);
    destroy_semaphores();

    free_shared_memory(shared_data);
}

int main(int argc, char *args[])
{
    processes_t arguments = parse_arguments(argc, args);
    if (!arguments.valid)
        return EXIT_FAILURE;

    int shared_mem_id = shmget(230818, sizeof(shared_data_t), 0666 | IPC_CREAT);
    if (shared_mem_id < 0)
        ERROR_EXIT("Could not allocate shared memory!\n", EXIT_FAILURE);

    shared_data = shmat(shared_mem_id, NULL, 0);
    if (shared_data == (shared_data_t *) -1)
        ERROR_EXIT("Could not allocate shared memory!\n", EXIT_FAILURE);
    shared_data->shm_id = shared_mem_id;

    if (initialize_shared_memory())
        return EXIT_FAILURE;

    int initialized = initialize_semaphores();
    if (initialized)
    {
        free_shared_memory(shared_data);
        return EXIT_FAILURE;
    }


    if (!create_forks(&arguments))
        wait_for_child_processes();

    free_initialized(0);

    return EXIT_SUCCESS;
}
