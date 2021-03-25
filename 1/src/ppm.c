// primes.h
// Řešení IJC-DU1, příklad b), 10.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Modul slúžiaci pre načítanie ppm obrázka.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ppm.h"
#include "error.h"

// Function allocates memory for image struct. It returns NULL if fails.
struct ppm* allocate_struct(size_t size_x, size_t size_y)
{
    unsigned image_size = 3 * size_x * size_y;

    if (image_size > PPM_IMAGE_MAX_SIZE)
    {
        warning_msg("Nepovolená velikost souboru!");
        return NULL;
    }

    struct ppm *image = malloc(sizeof(struct ppm) + image_size);
    if (!image)
    {
        warning_msg("Nepodařila se alokovat paměť!");
        return NULL;
    }

    image->xsize = size_x;
    image->ysize = size_y;

    return image;
}

// Function reads binary data of image colors. It returns NULL if fails.
bool read_bin_data(struct ppm *image, FILE *image_file)
{
    unsigned image_bytes = image->xsize * image->ysize * 3;

    size_t readed = fread(image->data, sizeof(char), image_bytes, image_file);

    if (!readed)
        warning_msg("Soubor se nepodařilo přečíst!");
    else if (readed < image_bytes)
        warning_msg("Soubor neobsahuje dostatek dat!");
    else if (fgetc(image_file) != EOF)
        warning_msg("Soubor obsahuje víc obrazových dat, než je povoleno!");
    else
        return true;

    return false;
}


#define CORRECT_ERR_RETURN(file) {fclose(file);return NULL;}
// Function is responsible for whole process of reading file.
struct ppm* ppm_read(const char *filename)
{
    FILE *image_file = fopen(filename, "rb");

    if (!image_file)
    {
        warning_msg("Nepodařilo se otevřít soubor!");
        return NULL;
    }

    size_t size_x = -1;
    size_t size_y = -1;
    int deph = -1;  // using int because at specific architectures CHAR could have 7bites so 255 won't fit into it
    fscanf(image_file, "P6 %zu %zu %d ", &size_x, &size_y, &deph);   // there is space at the end to catch whitespaces in front of binary data

    if (size_x <= 0 || size_y <= 0 || (deph <= 0 || deph > 255))
    {
        warning_msg("Nesprávna hlavička souboru!");
        CORRECT_ERR_RETURN(image_file);
    }

    struct ppm *image = allocate_struct(size_x, size_y);
    if (!image)
        CORRECT_ERR_RETURN(image_file);

    if (!read_bin_data(image, image_file))
    {
        ppm_free(image);
        CORRECT_ERR_RETURN(image_file);
    }

    fclose(image_file);
    return image;
}
#undef CORRECT_ERR_RETURN

void ppm_free(struct ppm *p)
{
    free(p);
}
