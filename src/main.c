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

int file_type(Elf *file)
{
  Elf_Kind ek = elf_kind (file);
  char *type;
  int ret = EXIT_SUCCESS;

  if (ek == ELF_K_AR)
    type = "shared library";
  else if (ek == ELF_K_ELF)
    type = "elf";
  else {
    type = "??? unrecognized, maybe text data";
    ret = EXIT_FAILURE;
  }

  if (ret == EXIT_SUCCESS && !elf64_newehdr(file))
  {
      fprintf(stderr, "Only 64-bit ELF files are supported\n");
      ret = EXIT_FAILURE;
  }

  printf("File type: %s32\n", type);
  return ret;
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
    file_type(file); // TODO: check it

    elf_end(file);
  }else
    fprintf(stderr, "Could not load file by elf_begin()\n");


  close(fd);
  return 0;
}
