// error.c
// Řešení IJC-DU1, příklad b), 10.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
// Modul pre výpis chýb. Samotny výpis by sa mohol presunúť do samostatnej funkcie, kde by sa parametrom predávali nepovinné
// parametre z warning_msg() a error_msg() a tam by ostal len va_start() ale keďže ide len o pár riadkov tak tá duplicita až tak nevadí.

#include "error.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// Function prints warning message to standard error output
void warning_msg(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
}

// Function prints message to standard error output and then exits the whole program.
void error_exit(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(1);
}
