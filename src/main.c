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

#include <libelf.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"


void check_args(int argc)
{
  if (argc != 2)
    ERROR_EXIT("Invalid arguments!");
}

/**
 * e_shoff member gives the byte offset from
the beginning of the file to the section header table; e_shnum tells how many entries the
section header table contains; e_shentsize gives the size in bytes of each entry.
 */

int open_file(char *filename)
{
  int fd = open(filename, O_RDONLY);
  if (fd < 0)
    ERROR_EXIT("Could not open file!");

  return fd;
}

int initialize_elf()
{
  if (elf_version(EV_CURRENT) == EV_NONE)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}



int main(int argc, char *args[])
{
  check_args(argc);
  int fd = open_file(args[1]);

  initialize_elf();
  Elf *file = elf_begin(fd, ELF_C_READ, NULL);
  //elf_errmsg ( -1);
  if (file)
  {

    elf_end(file);
  }else
    fprintf(stderr, "Could not load file by elf_begin()\n");


  close(fd);
  return 0;
}
