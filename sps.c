//
// Created by enhaut on 17. 11. 2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define print_error(...) fprintf(stderr, __VA_ARGS__ "\n")
#define MINIMAL_ARGUMENTS_COUNT 3
#define INITIAL_CELL_SIZE 50        // initial size of cells, it is dynamically allocated but there must be a starting point
#define CELL_SIZE_MULTIPLIER 2

#define MAXIMUM_COMMANDS_COUNT 1000
#define MAXIMUM_COMMAND_LENGTH 1000

#define SELECTION_COMMANDS_DELIMITER ","

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

typedef struct {
    table_index starting_row;
    table_index starting_cell;
    table_index ending_row;
    table_index ending_cell;
}CellsSelector;



bool string_compare(char *first, char *second)
{
    return strcmp(first, second) == 0;
}

void copy_to_array(char *destination, char *source, size_t how_much)
{
    strncpy(destination, source, how_much);
    destination[how_much] = '\0';
}

// Function will convert string number to float
bool get_numeric_cell_value(char *column, float *value)
{
    errno = 0;
    char *result;
    *value = strtof(column, &result);

    if (errno != 0 || (result != NULL && result[0] != '\0'))
        return false;
    return true;
}


bool provided_minimal_amount_of_arguments(int arg_count)
{
    if (arg_count >= MINIMAL_ARGUMENTS_COUNT)
        return false;
    print_error("Invalid arguments!");
    return true;
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

long get_file_size(FILE *file)
{
    fseek(file, 0, SEEK_END);   // seek to end of file
    long size = ftell(file);        // save last position
    rewind(file);                   // go back to the file beginning
    return size;
}

void copy_file_to_array(FILE *source, char *destination)
{
    int loaded_character;
    long position = 0;
    while ((loaded_character = getc(source)) != EOF)
    {
        destination[position] = (char) loaded_character;
        position++;
    }
    destination[position] = '\0';
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

unsigned short count_commands(char *commands, char commands_delimiter)
{
    int count = 0;
    size_t commands_length = strlen(commands);
    bool inside_quotation = false;
    for (size_t position = 0; position < commands_length; position++)
    {
        char loaded_character = commands[position];
        if (loaded_character == '\"')
            inside_quotation = inside_quotation ? false : true;

        if (!inside_quotation && loaded_character == commands_delimiter)
            count++;

        if (count == MAXIMUM_COMMANDS_COUNT + 1)    // dont need to count commands anymore, limit has been just reached
            return MAXIMUM_COMMANDS_COUNT + 1;
    }
    /*it s counting from 0, so we have to add 1, but only in case there are commands defined and
     * delimiter is not \n because it would count last empty line*/
    if (commands_length && commands_delimiter != '\n')
        count++;
    return count;
}

int valid_commands_argument(int arg_count, char *arguments[])
{
    int commands_argument_index = arg_count - 2;    // commands argument have to be 2. from the end
    char *commands_arguments = arguments[commands_argument_index];

    unsigned short commands_count;

    if (strstr(commands_arguments, "-c") == commands_arguments)     // -c have to be at beginning of argument
    {
        char *file_name = commands_arguments + 2;   // move pointer behind -c in argument
        FILE *commands_file = file_loader(file_name, "r");
        if (!commands_file)
            return EXIT_FAILURE;

        long file_size = get_file_size(commands_file);
        char file_content[file_size + 1];   // +1 for \0
        copy_file_to_array(commands_file, file_content);
        fclose(commands_file);
        commands_count = count_commands(file_content, '\n');
    }else
        commands_count = count_commands(commands_arguments, ';');


    if (!commands_count || commands_count > MAXIMUM_COMMANDS_COUNT)
    {
        print_error("Invalid commands!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
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

Table * initialize_table(TableSize dimensions, int *result)
{
    Table *table = (Table *)malloc(sizeof(Table));
    if (!table) {
        *result = EXIT_FAILURE;
        return table;
    }

    TableRow  **rows = (TableRow **)malloc(sizeof(TableRow *) * dimensions.rows);
    table->rows = rows;
    if (!rows) {
        *result = EXIT_FAILURE;
        return table;
    }

    for (table_index row_index = 0; row_index < dimensions.rows; row_index++)
    {
        TableRow *row = (TableRow *)malloc(sizeof(TableRow));
        rows[row_index] = row;
        if (!row) {
            *result = EXIT_FAILURE;
            return table;
        }
        row->cells = NULL;

        char **row_cells = (char **)malloc(sizeof(char *) * dimensions.columns);    // TODO: valg hlasi nealokovanu pamat ak ten malloc failne
        row->cells = row_cells;
        if (!row_cells) {
            *result = EXIT_FAILURE;
            return table;
        }

        for (table_index cell_index = 0; cell_index < dimensions.columns; cell_index++)
             row_cells[cell_index] = NULL;
    }
    return table;
}

// Function will resize (* CELL_SIZE_MULTIPLIER) cell array to make space for new characters
int resize_cell_if_needed(table_index position, table_index *cell_size, char **cell)
{
    if (position < *cell_size)
        return EXIT_SUCCESS;

    (*cell_size) *= CELL_SIZE_MULTIPLIER;
    char *larger = (char *) realloc(*cell, sizeof(char) * (*cell_size) + 1);
    if (!larger)
        return EXIT_FAILURE;

    *cell = larger;
    return EXIT_SUCCESS;
}

// Function will realloc cell array to used bytes to save memory.
void dealloc_unused_cell_part(char **cell, table_index size)
{
    char *reallocated_cell = (char *) realloc(*cell, size + 1);  // resize array to allocate needed memory only, +1 for \0
    if (reallocated_cell)   // in case, realloc fails, bigger cell (already allocated) will be used
        *cell = reallocated_cell;
}

char * load_table_cell(FILE *table_file, char *delimiters, bool *last_cell)
{
    table_index cell_length = INITIAL_CELL_SIZE;
    char *cell = (char *) malloc(sizeof(char) * cell_length + 1);
    if (!cell)
        return NULL;
    table_index position = 0;
    bool inside_quotation = false;  // used to prevent counting delimiters inside " " block
    int loaded_character;
    int character_before;

    while ((loaded_character = getc(table_file)) != EOF)
    {
        if (loaded_character == '\"' && character_before != '\\')
            inside_quotation = inside_quotation ? false : true;

        if ((!inside_quotation && character_before != '\\' && is_character_delimiter(delimiters, loaded_character)) || loaded_character == '\n')
            break;

        if (resize_cell_if_needed(position, &cell_length, &cell)) {
            free(cell);
            return NULL;
        }

        cell[position] = (char) loaded_character;
        position++;
        character_before = loaded_character;
    }

    if (loaded_character == '\n')
        *last_cell = true;

    dealloc_unused_cell_part(&cell, position);
    cell[position] = '\0';

    return cell;
}

char * get_empty_cell()
{
    char *cell = (char *) malloc(sizeof(bool));     // allocating bool size to save memory
    if (!cell)
        return NULL;

    cell[0] = '\0';
    return cell;
}

int fill_with_empty_cells(Table *table, TableSize size, table_index row, table_index column)
{
    for (table_index column_to_add = column + 1; column_to_add < size.columns; column_to_add++)
    {
        char *empty_cell = get_empty_cell();
        if (!empty_cell)
        {
            print_error("Could not allocate memory for table cells!");
            return EXIT_FAILURE;
        }
        table->rows[row]->cells[column_to_add] = empty_cell;
    }
    return EXIT_SUCCESS;
}

int load_table(FILE *table_file, Table *table, char *delimiters, TableSize size)
{
    for (table_index row = 0; row < size.rows; row++) {
        bool last_cell = false;
        for (table_index column = 0; column < size.columns; column++)
        {
            char *cell = load_table_cell(table_file, delimiters, &last_cell);
            if (!cell)
            {
                print_error("Could not allocate memory for table cells!");
                return EXIT_FAILURE;
            }
            table->rows[row]->cells[column] = cell;

            if (last_cell)
            {
                if (fill_with_empty_cells(table, size, row, column))
                    return EXIT_FAILURE;
                break;
            }
        }
    }

    rewind(table_file); // back to the start of file
    return EXIT_SUCCESS;
}

void destruct_table(Table *table, TableSize size)
{
    if (!table)         // allocation of table failed, so nothing remaining to free
        return;
    if (!table->rows)   // allocation of rows array failed, there is just table to free
    {
        free(table);
        return;
    }

    for (table_index row_index = 0; row_index < size.rows; row_index++)
    {
        if (!table->rows[row_index])    // in case, allocation of row failed, there are no cells
            break;

        if (table->rows[row_index]->cells)  // deallocating allocated cells only
        {
            for (table_index cell_index = 0; cell_index < size.columns; cell_index++)
            {
                if (table->rows[row_index]->cells[cell_index])
                    free(table->rows[row_index]->cells[cell_index]);
            }
            free(table->rows[row_index]->cells);
        }
        free(table->rows[row_index]);   // deallocate row
    }
    free(table->rows);  // deallocate rows storing array
    free(table);        // deallocate empty table
}

/* Function will return size of savable table. That will remove empty columns at the end of table. */
TableSize get_savable_table_size(Table *table, TableSize size)
{
    TableSize savable_size;
    savable_size.columns = size.columns;
    savable_size.rows = size.rows;

    for (table_index column = size.columns - 1; column; column--)
    {
        bool can_skip = true;
        for (table_index row = 0; row < size.rows; row++)
        {
            if (table->rows[row]->cells[column] && strlen(table->rows[row]->cells[column]))
            {
                can_skip = false;
                break;
            }
        }
        if (can_skip)
            savable_size.columns--;
        else
            break;  // if the actual column is not removable, columns before could not be removed too
    }
    return savable_size;
}

void print_table(Table *table, TableSize size)
{
    size = get_savable_table_size(table, size);
    for (table_index row = 0; row < size.rows; row++)
    {
        for (table_index column = 0; column < size.columns; column++) {
            printf("%s", table->rows[row]->cells[column]);
            if (column < (size.columns - 1))
                printf(",");
        }
        printf("\n");
    }
}

void save_table(Table *table, FILE *table_file, TableSize size, char delimiter)
{
    freopen(NULL, "w", table_file);     // file is opened in r mode, so reopen it to rewrite content
    size = get_savable_table_size(table, size);

    for (table_index row = 0; row < size.rows; row++)
    {
        for (table_index column = 0; column < size.columns; column++)
        {
            fprintf(table_file, "%s", table->rows[row]->cells[column]);
            if (column < (size.columns - 1))    // write delimiter behind not last columns
                fputc(delimiter, table_file);
        }
        fputc('\n', table_file);
    }
}

char *get_command_from_argument(char **cell_start, bool first_command, unsigned short *command_length)
{
    char *command_end = *cell_start;
    size_t remaining_commands_size = strlen(*cell_start);
    bool inside_quotation = false;
    if (!remaining_commands_size)
        return NULL;
    if (command_end[0] == ';' || command_end[0] == '\n')
        command_end++;

    for (unsigned int position = 0; position < remaining_commands_size; position++)
    {
        if ((*cell_start)[position] == '\"' && ((!first_command || position) && (*cell_start)[position - 1] != '\\'))
            inside_quotation = inside_quotation ? false : true;

        if ((!inside_quotation && (*cell_start)[position] == ';') || (*cell_start)[position] == '\n')
            break;

        command_end++;  // moving command end to last character of command
        (*command_length)++;

        if (*command_length > MAXIMUM_COMMAND_LENGTH)
        {
            print_error("Selector is too long!");
            break;
        }
    }
    return command_end;
}

void initialize_selector(CellsSelector *selector, TableSize size)
{
    selector->starting_row = 0;
    selector->starting_cell = 0;

    selector->ending_row = size.rows;
    selector->ending_cell = size.columns;
}

table_index * get_selector_from_index(unsigned short index, CellsSelector *selector)
{
    table_index *save_to = &selector->starting_row;
    if (index == 1)
        save_to = &selector->starting_cell;
    else if (index == 2)
        save_to = &selector->ending_row;
    else if (index == 3)
        save_to = &selector->ending_cell;
    return save_to;
}

unsigned short process_normal_selector(CellsSelector *selector, char *command)
{
    char *selector_value;
    short selector_index = -1;
    selector_value = strtok(++command, SELECTION_COMMANDS_DELIMITER);    // moving pointer behind [

    while (selector_value != NULL && selector_index++ < 4)   // there could be up to 4 selectors (R1, C1, R2, C2)
    {
        table_index *save_to = get_selector_from_index(selector_index, selector);

        if (strchr(selector_value, '_') == selector_value) {
            *save_to = 0;
            continue;
        }

        char *raw_selector = selector_value;
        char *remaining = NULL;
        table_index value = 0;
        value = strtoull(selector_value, &remaining, 10);

        selector_value = strtok(NULL, SELECTION_COMMANDS_DELIMITER);
        if ((!selector_value && !strchr(raw_selector, ']')) ||                   // last selector should contains ]
            (remaining[0] != '\0' && !(!selector_value && remaining[0] == ']')) ||  // invalid selector detection but exclude ending selector with trailing ]
            ((selector_index == 0 || selector_index == 2) && !selector_value) ||    // invalid selectors count
            !value)
        {
            print_error("Invalid selecor!");
            return EXIT_FAILURE;
        }
        *save_to = value - 1;   // rows & columns are indexed from 0
    }
    return EXIT_SUCCESS;
}

// Function returns selector length. Minimal length of selector is 3 characters - [_]. So 0-3 can be used as error codes.
unsigned short process_selector(CellsSelector *selector, char *command)
{
    if (command[0] != '[')      // first character is not a selector
        return EXIT_SUCCESS;

    unsigned short result;

    result = process_normal_selector(selector, command);
    if (result == EXIT_FAILURE)
        return EXIT_FAILURE;

    return result;
}

int process_commands(Table *table, TableSize size, int arg_count, char **arguments)
{
    (void)table;
    (void)size;

    char *command_start = arguments[arg_count - 2];
    char *command_end = command_start;
    CellsSelector selected;
    initialize_selector(&selected, size);

    bool first_command = true;
    unsigned short command_length = 0;

    while ((command_end = get_command_from_argument(&command_end, first_command, &command_length)) != NULL)
    {
        if (command_length > MAXIMUM_COMMAND_LENGTH)
            return EXIT_FAILURE;

        char command[command_length + 1];
        copy_to_array(command, command_start, command_length);
        unsigned short selector_parsing_result = process_selector(&selected, command);
        if (selector_parsing_result == EXIT_FAILURE)
            return EXIT_FAILURE;

        first_command = false;
        command_start = command_end;
        command_length = 0;
    }
    return EXIT_SUCCESS;
}

int main(int arg_count, char *arguments[])
{
    if (provided_minimal_amount_of_arguments(arg_count))
        return EXIT_FAILURE;

    if (valid_commands_argument(arg_count, arguments))
        return EXIT_FAILURE;

    int is_defined_delimiter = defined_delimiter(arg_count, arguments);
    if (is_defined_delimiter == -1)
        return EXIT_FAILURE;

    char default_delimiter[] = " ";
    char *delimiter = default_delimiter;

    if (is_defined_delimiter)
        delimiter = arguments[2];


    FILE *table_file = file_loader(arguments[get_table_filename_index(arg_count)], "r+");
    if (!table_file)
        return EXIT_FAILURE;

    TableSize size;
    get_table_size(table_file, delimiter, &size);

    int successful_initialization = EXIT_SUCCESS;
    Table *table = initialize_table(size, &successful_initialization);

    if (!successful_initialization)
    {
        int successfully_loaded = load_table(table_file, table, delimiter, size);
        if (!successfully_loaded)
        {
            print_table(table, size);
            process_commands(table, size, arg_count, arguments);
            save_table(table, table_file, size, delimiter[0]);
        }
    }

    destruct_table(table, size);
    fclose(table_file);
    return EXIT_SUCCESS;
}
