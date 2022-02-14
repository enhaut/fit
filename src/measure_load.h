//
// Created by Samuel Dobron on 14.02.2022.
//

#ifndef IPK_PROJ1_MEASURE_LOAD_H
#define IPK_PROJ1_MEASURE_LOAD_H

#define ULLI unsigned long long int
typedef struct load{
    ULLI user;
    ULLI nice;
    ULLI system;
    ULLI idle;
    ULLI iowait;
    ULLI irq;
    ULLI softirq;
    ULLI steal;
    ULLI guest;
    ULLI guest_nice;
}load_t;

#define SLEEP_FOR 1
int calculate_load();
#endif//IPK_PROJ1_MEASURE_LOAD_H
