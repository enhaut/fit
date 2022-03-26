/**
 * ELF reader
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


int print_sections(Elf *file, GElf_Phdr *program_header)
{
  GElf_Shdr section_header;
  size_t sec_index;
  Elf_Scn *section = NULL;
  char *name;

  if (elf_getshdrstrndx(file, &sec_index))
    return EXIT_FAILURE;

  while ((section = elf_nextscn(file, section)) != NULL)
  {
    if (gelf_getshdr(section, &section_header) != &section_header)
      return EXIT_FAILURE;

    if ((name = elf_strptr(file, sec_index, section_header.sh_name)) == NULL)
      return EXIT_FAILURE;

    if (section_header.sh_addr && section_header.sh_size &&
        (program_header->p_offset <= section_header.sh_offset &&
         section_header.sh_offset < (program_header->p_offset + program_header->p_memsz)))
       printf ( "%s ", name);
  }
  return EXIT_SUCCESS;
}

int print_segment(Elf *file, GElf_Phdr *seg, int i)
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
  if (print_sections(file, seg))
    return EXIT_FAILURE;

  putc('\n', stdout);
  return EXIT_SUCCESS;
}

int segments(Elf *file)
{
  size_t segments_no;
  if (elf_getphdrnum(file, &segments_no))
    return EXIT_FAILURE;
  printf("Segments: %lu\n", segments_no);
  printf("%s\t\t%-17s %-7s %s\n", "Segment", "Type", "Perm", "Sections");

  GElf_Phdr prog_header;
  for (int i = 0; i < (int)segments_no; ++i)
  {
    if (gelf_getphdr(file, i, &prog_header) == NULL)
      return EXIT_FAILURE;
    if (print_segment(file, &prog_header, i))
      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int main(int argc, char *args[])
{
  check_args(argc);
  int fd = open_file(args[1]);

  if (initialize_elf())
    fprintf(stderr, "Could not initialize ELF\n");
  else
  {
    Elf *file = elf_begin(fd, ELF_C_READ, NULL);
    if (file)
    {
      if (file_type(file))
        fprintf(stderr, "Could not get file type!\n");
      else
      {
        if(segments(file))
          fprintf(stderr, "Could not print segments!\n");

        elf_end(file);
      }
    }else
      fprintf(stderr, "Could not load file by elf_begin()\n");
  }
  close(fd);
  return 0;
}
