// wordcount-.cc
// Použijte: g++ -std=c++11 -O2
// Příklad použití STL kontejneru unordered_map<>
// Program počítá četnost slov ve vstupním textu,
// slovo je cokoli oddělené "bílým znakem"

#include "htab.h"
#include "io.h"
#define WORD_LIMIT 128
#define HASH_TABLE_SIZE 128

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
