// wordcount-.cc
// Použijte: g++ -std=c++11 -O2
// Příklad použití STL kontejneru unordered_map<>
// Program počítá četnost slov ve vstupním textu,
// slovo je cokoli oddělené "bílým znakem"

#include "htab.h"
#include "io.h"
#define WORD_LIMIT 128
/*
 * Hash table size should be at least 1.3x bigger than number of maximum
 * values also it should be prime number.
 * I don't know the number of words in provided files, so i just choose 1117.
 * The more memory it uses the faster it is, so we have to find a compromise.
*/
#define HASH_TABLE_SIZE 1117

#ifdef HASHTEST
// djb2 hash algo from http://www.cse.yorku.ca/~oz/hash.html
size_t htab_hash_function(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
#endif

void print_result(htab_pair_t *pair)
{
    printf("%s\t%d\n", pair->key, pair->value);
}

int main(void)
{
    htab_t *table = htab_init(HASH_TABLE_SIZE);
    if (!table)
    {
        fprintf(stderr, "Could not initialize table!");
        return 1;
    }

    char word[WORD_LIMIT] = {0};
    while (read_word(word, WORD_LIMIT, stdin) != EOF)
    {
        htab_pair_t *pair = htab_lookup_add(table, word);
        if (!pair)
            continue;

        pair->value++;
    }

#ifdef MOVETEST
    htab_t *new_table = htab_move(HASH_TABLE_SIZE, table);
    if (!new_table)
    {
        fprintf(stderr, "Could not move data!");
        return 1;
    }
    htab_free(table);
    table = new_table;
#endif

    htab_for_each(table, print_result);
    htab_free(table);
    return 0;
}
