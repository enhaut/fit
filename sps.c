//
// Created by enhaut on 17. 11. 2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define print_error(...) fprintf(stderr, __VA_ARGS__ "\n")
#define MINIMAL_ARGUMENTS_COUNT 3
#define INITIAL_CELL_SIZE 50        // initial size of cells, it is dynamically allocated but there must be a starting point

typedef unsigned long long table_index;     // rows and columns have no limit, so I am using ull

typedef struct {
    table_index rows;
    table_index columns;
}TableSize;

typedef struct {
    char **cells;
}TableRow;

typedef struct {
    TableRow **rows;
}Table;


bool string_compare(char *first, char *second)
{
    return strcmp(first, second) == 0;
}

bool provided_minimal_amount_of_arguments(int arg_count)
{
    if (arg_count >= MINIMAL_ARGUMENTS_COUNT)
        return false;
    print_error("Invalid arguments!");
    return true;
}

// Returns -1 for invalid delimiter -> exit, 0 for no delimiter defined, 1 for defined delimiter
int defined_delimiter(int arg_count, char *arguments[])
{
    bool is_set_d_argument = string_compare(arguments[1], "-d");

    if (arg_count == 4 || (arg_count == 5 &&
                          (((is_set_d_argument && strlen(arguments[2]) == 0) || // delimiter value is empty
                            !is_set_d_argument) ||                                 // 2. argument is not -d
                            (strchr(arguments[2], '"') || strchr(arguments[2], '\\')   // forbidden chars in delimiter
                            ))))
    {
        print_error("Invalid delimiter!");
        return -1;
    }else if (arg_count == 5 && is_set_d_argument)
        return 1;

    return 0;
}

bool is_character_delimiter(char *delimiters, int delimiter)
{
    if (strchr(delimiters, delimiter))
        return true;
    return false;
}

int get_table_filename_index(int arg_count)
{
    return arg_count - 1;   // filename is always last argument
}

FILE *file_loader(char *filename, char *mode)
{
    FILE *opened_file = fopen(filename, mode);
    if (!opened_file) {
        print_error("Could not open file!");
        return NULL;
    }
    return opened_file;
}

void get_table_size(FILE *table_file, char *delimiters, TableSize *size)
{
    table_index rows = 0;
    table_index columns = 0;
    table_index row_columns = 0;
    int loaded_character;
    int character_before;
    bool inside_quotation = false;  // used to prevent counting delimiters inside " " block

    while ((loaded_character = getc(table_file)) != EOF)
    {
        if (loaded_character == '\"') {
            inside_quotation = inside_quotation ? false : true;
            continue;
        }

        if (!inside_quotation && is_character_delimiter(delimiters, loaded_character))
            row_columns++;

        if (loaded_character == '\n' && loaded_character != character_before) {
            //                             ^ -- prevent counting empty lines
            rows++;
            if (row_columns > columns)
                columns = row_columns;  // save the count of cells in the biggest row
            row_columns = 0;            // new row reached, reset counter
        }
        character_before = loaded_character;
    }
    size->rows = rows;  // TODO: counting rows in file with no empty line at the EOF
    size->columns = rows ? columns + 1 : 0; // the counter counts delimiters only so +1 to add missing column

    rewind(table_file); // back to the start of file
}

Table * initialize_table(TableSize dimensions)
{
    // TODO: checking malloc result
    Table *table = (Table *)malloc(sizeof(Table));

    TableRow  **rows = (TableRow **)malloc(sizeof(TableRow *) * dimensions.rows);

    for (table_index row_index = 0; row_index < dimensions.rows; row_index++)
    {
        TableRow *row = (TableRow *)malloc(sizeof(TableRow));
        char **row_cells = (char **)malloc(sizeof(char *) * dimensions.columns);
        for (table_index cell_index = 0; cell_index < dimensions.columns; cell_index++)
             row_cells[cell_index] = NULL;

        row->cells = row_cells;
        rows[row_index] = row;
    }
    table->rows = rows;
    return table;
}

int resize_cell_if_needed(table_index position, table_index *cell_size, char **cell)
{
    if (position < *cell_size)
        return EXIT_SUCCESS;

    (*cell_size) *= 2;
    char *larger = (char *) realloc(*cell, sizeof(char) * (*cell_size) + 1);
    if (!larger)
    {
        free (cell);
        return EXIT_FAILURE;
    }

    *cell = larger;
    return EXIT_SUCCESS;
}

char * load_table_cell(FILE *table_file, char *delimiters)
{
    table_index cell_length = INITIAL_CELL_SIZE;
    char *cell = (char *) malloc(sizeof(char) * cell_length + 1);
    if (!cell)
        return NULL;
    table_index position = 0;
    bool inside_quotation = false;  // used to prevent counting delimiters inside " " block
    int loaded_character;

    while ((loaded_character = getc(table_file)) != EOF)
    {
        if (loaded_character == '\"')
            inside_quotation = inside_quotation ? false : true;

        if ((!inside_quotation && is_character_delimiter(delimiters, loaded_character)) || loaded_character == '\n')
            break;

        if (resize_cell_if_needed(position, &cell_length, &cell))
            return NULL;

        cell[position] = (char) loaded_character;
        position++;
    }

    char *reallocated_cell = (char *) realloc(cell, position + 1);  // resize array to allocate needed memory only, +1 for \0
    if (reallocated_cell)   // in case, realloc fails, bigger cell (already allocated) will be used
        cell = reallocated_cell;

    cell[position] = '\0';
    return cell;
}

int load_table(FILE *table_file, Table *table, char *delimiters, TableSize size)
{
    for (table_index row = 0; row < size.rows; row++) {
        for (table_index column = 0; column < size.columns; column++) {
            char *cell = load_table_cell(table_file, delimiters);
            if (!cell)
            {
                print_error("Could not allocate memory for table cells!");
                return EXIT_FAILURE;
            }
            table->rows[row]->cells[column] = cell;
        }
    }

    rewind(table_file); // back to the start of file
    return EXIT_SUCCESS;
}

void destruct_table(Table *table, TableSize size)
{
    for (table_index row_index = 0; row_index < size.rows; row_index++)
    {
        for (table_index cell_index = 0; cell_index < size.columns; cell_index++)
        {
            if (table->rows[row_index]->cells[cell_index] != NULL)
                free(table->rows[row_index]->cells[cell_index]);
        }
        free(table->rows[row_index]->cells);
        free(table->rows[row_index]);
    }
    free(table->rows);  // remove rows storing array
    free(table);
}

void print_table(Table *table, TableSize size)
{
    for (table_index row = 0; row < size.rows; row++)
    {
        for (table_index column = 0; column < size.columns; column++) {
            printf("%s ", table->rows[row]->cells[column]);
        }
        printf("\n");
    }
}

int main(int arg_count, char *arguments[])
{
    if (provided_minimal_amount_of_arguments(arg_count))
        return EXIT_FAILURE;

    int is_defined_delimiter = defined_delimiter(arg_count, arguments);
    if (is_defined_delimiter == -1)
        return EXIT_FAILURE;

    char default_delimiter[] = " ";
    char *delimiter = default_delimiter;

    if (is_defined_delimiter)
        delimiter = arguments[2];


    FILE *table_file = file_loader(arguments[get_table_filename_index(arg_count)], "r");
    if (!table_file)
        return EXIT_FAILURE;

    TableSize size;
    get_table_size(table_file, delimiter, &size);

    Table *table = initialize_table(size);
    int load_result = load_table(table_file, table, delimiter, size);
    if (load_result)
        return EXIT_FAILURE;
    print_table(table, size);

    destruct_table(table, size);
    printf("Hotovo");
    fclose(table_file);
    return EXIT_SUCCESS;
}
