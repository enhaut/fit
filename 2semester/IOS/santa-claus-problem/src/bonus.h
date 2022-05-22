// bonus.h
// 
// Author: Samuel Dobroň (xdobro23), FIT VUTBR
// Compiled: gcc 10.2.1
// 2. 5. 2021

#ifndef SANTA_CALAUS_PROBLEM_BONUS_H
#define SANTA_CALAUS_PROBLEM_BONUS_H
int generate_more_elves(shared_data_t *data, processes_t *arguments);
void generate_elves_handler(int signum);
void stop_generating_elves(int signum);
#endif //SANTA_CALAUS_PROBLEM_BONUS_H
