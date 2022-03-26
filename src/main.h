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

#define SET_SEG_TYPE(type, string) do{              \
  if (type >= PT_LOOS && type <= PT_HIOS)           \
    (string) = "LOOS(env. spec.)";                  \
  else if (type >= PT_LOPROC && type <= PT_HIPROC)  \
    (string) = "LOPROC(cpu spec.)";                 \
  else {                                            \
      switch (type) {                               \
        case PT_NULL:                               \
          (string) = "NULL(unused)";                \
          break;                                    \
        case PT_LOAD:                               \
          (string) = "LOAD";                        \
          break;                                    \
        case PT_DYNAMIC:                            \
          (string) = "DYNAMIC";                     \
          break;                                    \
        case PT_INTERP:                             \
          (string) = "INTERP";                      \
          break;                                    \
        case PT_NOTE:                               \
          (string) = "NOTE";                        \
          break;                                    \
        case PT_SHLIB:                              \
          (string) = "RESERVED";                    \
          break;                                    \
        case PT_PHDR:                               \
          (string) = "PHDR";                        \
          break;                                    \
        default:                                    \
          (string) = "???";                         \
      }                                             \
  }                                                 \
  }while(0)

#define SET_PERMISSIONS(to_print, flags) do{\
  if ((flags) & PF_R)                       \
      (to_print)[0] = 'R';                  \
  if ((flags) & PF_W)                       \
      (to_print)[1] = 'W';                  \
  if ((flags) & PF_X)                       \
      (to_print)[2] = 'X';                  \
  }while(0)

#endif // IAN_MAIN_H
