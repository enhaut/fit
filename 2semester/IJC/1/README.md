# Bitset array - project 1

## Usage
As all my other projects, also this one has `Makefile`. Just use make:
```
make run
```
to run prime numbers calculation.  
```
make steg-decode
./steg-decode du1-obrazek.ppm
```
to run decoder.

## Assignment
### Bitset array (up to 7 points)
To interface `bitset.h` define structure array of bits:  
Types:
- `typedef <YOUR-TYPE> bitset_t` - structure of bit array
- `typedef unsigned long bitset_index_t` - type for index to item in array

Macros:  
- `bitset_create(name, size)` - defines and initializes variable `name`
  - be sure, your implementation supports both static and local arrays.
  - examples: `static bitset_create(p, 100); // p = array of 100 bits, initialized`, `bitset_create(q, 100000L);  // q = array of 100000 bits, initialized` and `bitset_create(q,-100);  // compilation error`
- `bitset_alloc(name, size)` - defines variable `name` in compatible way with array defined with `bitset_create` but heap needs to be used
  - `bitset_alloc(q, 100000L); // q = array of 100000 bits, initialized`
  - Use `assert` for maximum size check.
  - In case of allocation fail throw `bitset_alloc: Chyba alokace paměti` error
- `bitset_free(name)`
- `bitset_size(name)` - returns size of bit array (saved in array)
- `bitset_setbit(name, index, value)` - sets `index` bit to `value`
- `bitset_getbit(name, index)` - returns value of `index` bit in `name` array

For testing purposes, implement sieve of Eratosthenes for prime numbers calculation. Function `Eratosthenes()` have to
calculate prime numbers and prints it to the `stdout`.

### Decoder (up to 8 points)
Implement module `error.c` with interface defined in `error.h` which defines function:
- `void warning_msg(const char *fmt, ...)` 
- `void error_exit(const char *fmt, ...)`

These functions support same parameters as `print()`. It should print formatted error message to `stderr` and ends 
program.  
  
Implement module `ppm.c` with interface `ppm.h` where you define type:
```C
struct ppm {
    unsigned xsize;
    unsigned ysize;
    char data[];    // RGB bytes, total of 3*xsize*ysize
};
```
and functions:
- `struct ppm * ppm_read(const char * filename)` - it loads content of [PPM](https://en.wikipedia.org/wiki/Netpbm#File_formats) file to dynamically allocated structure.
  - in case invalid PPM file format, use your `warning_msg()` function and return `NULL`
- `void ppm_free(struct ppm *p)` - frees dynamically allocated memory from `ppm_read()`

Implement just binary variant `P6` of PPM with 256 colors support without comments inside:
```
"P6" <ws>+
<xsizetxt> <ws>+ <ysizetxt> <ws>+
"255" <ws>
<binary data, 3*xsize*ysize bytes RGB>
<EOF>
```

Use `ppm_read()` and `ppm_free()` functions in `steg-decode.c` file that implements message decoder from ppm file 
([du1-obrazek.ppm](src/du1-obrazek.ppm). The secret is sequence of `char`s that consists of bits saved as LSb on prime 
indexes starts at 29. Use your implementation of sieve of Eratosthenes for prime numbers calculation.


## Evaluation
15 / 15 points :)
