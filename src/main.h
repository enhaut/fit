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

#define TYPES_COUNT 12
char *seg_types[TYPES_COUNT] = {
    "NULL(unused)",
    "LOAD",
    "DYNAMIC",
    "INTERP",
    "NOTE",
    "RESERVED",
    "PHDR",
    "LOOS(env. spec.)",
    "HIOS",
    "LOPROC(cpu spec.)",
    "HIPROC",
    "UNSUPPORTED"  // keep as last entry
};

char *get_seg_type(Elf64_Word type)
{
  switch (type)
  {
    case 0x60000000:
      return seg_types[7];
    case 0x6fffffff:
      return seg_types[8];
    case 0x70000000:
      return seg_types[9];
    case 0x7fffffff:
      return seg_types[10];
    default:
      return (type >= 0 && type <= 6) ? seg_types[type] : seg_types[TYPES_COUNT - 1];
    }
}

#endif // IAN_MAIN_H
