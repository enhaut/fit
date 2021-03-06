// proj2.h
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 18. 4. 2021
#include <stdbool.h>
#include <stdio.h>
#include "semaphore.h"

#ifndef SANTA_CALAUS_PROBLEM_PROJ2_H
#define SANTA_CALAUS_PROBLEM_PROJ2_H

#define ERROR_EXIT(message,to_return) do{fprintf(stderr, message); return to_return;}while(0)

#define NE_LIMIT 1000
#define NR_LIMIT 20
#define ABOVE_0 0
#define ABOVE_EQUAL_0 -1
#define TIMER_LIMIT 1001    // +1 because of >= in checking func

typedef struct {
    int NE;
    int NR;
    int TE;
    int TR;
    bool valid;
}processes_t;


typedef struct {
    int shm_id;
    int reindeers;
    int waiting_elves;
    bool closed;
    int message_counter;
    FILE *log_file;
    int *child_pids;

    struct{
        sem_t santa;
        sem_t waiting_santa;
        sem_t reindeers;
        sem_t elves_in;
        sem_t elves_mutex;
        sem_t mutex;
        sem_t print;
#ifdef BONUS
        sem_t can_generate_elves;
#endif
    } sems;
}shared_data_t;

void correct_print(shared_data_t *data, const char *fmt, ...);
void delete_pid(int *pids, int pid);
void free_initialized(int signum);

#define CREATE_FORK(failed, pid_index, pids, fork_function, ...) do{                                                  \
                                                                    int fork_pid = fork();                            \
                                                                    pids[pid_index+1] = fork_pid;                     \
                                                                    if (fork_pid == 0)                                \
                                                                        exit(fork_function(__VA_ARGS__));             \
                                                                    else if(fork_pid < 0)                             \
                                                                        *failed = true;                               \
                                                                 }while(0)

#endif //SANTA_CALAUS_PROBLEM_PROJ2_H
