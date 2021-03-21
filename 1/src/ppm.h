// primes.h
// Řešení IJC-DU1, příklad b), 10.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//


#ifndef IJC_PPM_H
    #define IJC_PPM_H

    struct ppm {
        unsigned xsize;
        unsigned ysize;
        char data[];
    };

    struct ppm * ppm_read(const char *filename);
    void ppm_free(struct ppm *p);

    #define PPM_IMAGE_MAX_SIZE (1431655765)    // UINT_MAX/3
#endif //IJC_PPM_H
