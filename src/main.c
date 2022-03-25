/**
 * ian
 *
 * @file main.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */

#include "main.h"

void check_args(int argc, char *args[])
{
  if (argc != 2)
    ERROR_EXIT("Invalid arguments!");
}

/**
 * e_shoff member gives the byte offset from
the beginning of the file to the section header table; e_shnum tells how many entries the
section header table contains; e_shentsize gives the size in bytes of each entry.
 */

int main(int argc, char *args[])
{
  check_args(argc, args);




  return 0;
}
