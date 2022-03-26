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
#include <gelf.h>

#include "main.h"


void check_args(int argc)
{
  if (argc != 2)
    ERROR_EXIT("Invalid arguments!");
}

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

  printf("File type: %s64\n", type);
  return ret;
}


void print_sections(Elf *e, GElf_Phdr *program_header)
{
  GElf_Shdr shdr;
  size_t shstrndx;
  Elf_Scn *scn = NULL;
  char *name;

  elf_getshdrstrndx(e, &shstrndx);

  while (( scn = elf_nextscn (e, scn)) != NULL )
  {
    if (gelf_getshdr( scn, &shdr) != &shdr)
      printf("ERR");

    if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL)
      printf("ERROR2");

    if (shdr.sh_addr &&
        shdr.sh_size &&
        (program_header->p_offset <= shdr.sh_offset &&
         shdr.sh_offset < (program_header->p_offset + program_header->p_memsz)))
       printf ( "%s ", name);
  }

}

void print_segment(Elf *file, GElf_Phdr *seg, int i)
{
  char permissions[] = "---";
  SET_PERMISSIONS(permissions, seg->p_flags);

  char *type;
  SET_SEG_TYPE(seg->p_type, type);

  printf(
      "%03d\t\t%-17s %-7s ",
      i,
      type,
      permissions
  );
  print_sections(file, seg);

  putc('\n', stdout);
}

int segments(Elf *file)
{
  size_t segments_no;
  if (elf_getphdrnum(file, &segments_no))
    return EXIT_FAILURE;
  printf("Segments: %lu\n", segments_no);

  GElf_Phdr prog_header;
  for (int i = 0; i < (int)segments_no; ++i)
  {
    gelf_getphdr(file, i, &prog_header);  // TODO: check
    print_segment(file, &prog_header, i);
  }

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
    file_type(file); // TODO: check it
    segments(file);

    elf_end(file);
  }else
    fprintf(stderr, "Could not load file by elf_begin()\n");


  close(fd);
  return 0;
}
