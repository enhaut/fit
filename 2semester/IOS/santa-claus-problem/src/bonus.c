#include <stdatomic.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "proj2.h"
#include "elf.h"

atomic_bool generate_elves = true;
int generated_elves = 0;
shared_data_t *bonus_shared_data = NULL;

void generate_elves_handler(int signum)
{
    (void)signum;
    sem_post(&(bonus_shared_data->sems.can_generate_elves));
}

void stop_generating_elves(int signum)
{
    printf("generating has been stopped\n");
    printf("toasd: %d\n", generated_elves);
    (void)signum;
    generate_elves = false;

    /*RELEASING WAITING ELVES*/
    for (int i = 0 ; i < generated_elves; i++)
    {
        sem_post(&(bonus_shared_data->sems.elves_mutex));
        sem_post(&(bonus_shared_data->sems.mutex));
    }
    for (int i = 0; i < 3; i++)
        sem_post(&bonus_shared_data->sems.elves_in);
}

int fork_elves(shared_data_t *data, processes_t *arguments)
{
    srand(time(0));
    printf("elvo: %d\n", arguments->NE);
    int elves_to_fork = 1 + rand() % arguments->NE;
    int elves_before = data->child_pids[0];
    bool failed = false;

    /* REALLOCATING PIDS ARRAY TO MAKE SPACE FOR NEW FORK PIDS */
    int *reallocated = realloc(data->child_pids, sizeof(int) * (elves_before + elves_to_fork));
    if (!reallocated)
    {
        free_initialized(0);
        ERROR_EXIT("Creating forks has failed!\n", EXIT_FAILURE);
    }
    data->child_pids = reallocated;
    data->child_pids[0] = elves_before + elves_to_fork;
    for (int i = 0; i < elves_to_fork; i++)
        data->child_pids[elves_before + i] = 0;

    printf("creating %d new elves\n", elves_to_fork);
    /* GENERATING NEW FORKS */
    for (int i = 0; i < elves_to_fork && !failed && generate_elves; i++)
    {
        int elf_id = elves_before - arguments->NR - 1 + i;   // new elf id (elves # before forking - # of reindeers - santa + i)
        CREATE_FORK(&failed, elves_before + i, data->child_pids, elf, data, arguments, elf_id);
        generated_elves++;
    }
    if (failed)
    {
        free_initialized(0);
        ERROR_EXIT("Creating forks has failed!\n", EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int generate_more_elves(shared_data_t *data, processes_t *arguments)
{
    bonus_shared_data = data;
    int forking_failed = EXIT_SUCCESS;

    while(generate_elves && !forking_failed)
    {
        printf("waiting for sigusr1\n");
        sem_wait(&(data->sems.can_generate_elves));
        printf("generating\n");
        forking_failed = fork_elves(data, arguments);
    }
    return forking_failed;
}

