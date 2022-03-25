/**
 * ian
 *
 * @file main.h
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef IAN_MAIN_H
#define IAN_MAIN_H

#define ERROR_EXIT(message)               \
    do{                                   \
        fprintf(stderr, message "\n");    \
        exit(EXIT_FAILURE);               \
    }while(0)

#endif // IAN_MAIN_H
