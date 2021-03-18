// error.h
// Řešení IJC-DU1, příklad b), 10.3.2021
// Autor: Samuel Dobroň, FIT
// Přeloženo: gcc 10.2.1
//

#ifndef ERROR_H
    #define ERROR_H

    void warning_msg(const char *fmt, ...);
    void error_exit(const char *fmt, ...);
#endif