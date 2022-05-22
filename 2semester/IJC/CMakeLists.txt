cmake_minimum_required(VERSION 3.17)
project(ijc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(USE_INLINE)

include_directories(1/src)
include_directories(2)

add_executable(du1
        1/src/bitset.c
        1/src/bitset.h
        1/src/eratosthenes.c 1/src/eratosthenes.h 1/src/primes.c 1/src/error.c 1/src/error.h 1/src/ppm.c 1/src/ppm.h 1/src/steg-decode.c)

add_executable(du2
        2/htab.h
        2/htab_private.h
        2/htab_init.c
        2/htab_hash_function.c
        2/htab_free.c
        2/htab_clear.c
        2/htab_move.c
        2/htab_size.c
        2/htab_bucket_count.c
        2/htab_find.c
        2/htab_lookup_add.c
        2/htab_for_each.c
        2/htab_erase.c
        2/io.c
        2/io.h
        2/wordcount.c
        2/tail.c
        )
