//
// Created by enhaut on 17. 11. 2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <float.h>

#define print_error(...) fprintf(stderr, __VA_ARGS__ "\n")
#define MINIMAL_ARGUMENTS_COUNT 3
#define INITIAL_CELL_SIZE 50        // initial size of cells, it is dynamically allocated but there must be a starting point
#define CELL_SIZE_MULTIPLIER 2

#define MAXIMUM_COMMANDS_COUNT 1000
#define MAXIMUM_COMMAND_LENGTH 1000

#define SELECTION_COMMANDS_DELIMITER ","
#define USER_VARIABLES_COUNT 10

#define SUPPORTED_COMMANDS_COUNT 22

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

typedef unsigned short (*function_ptr)(); // pointer to function with no strict arguments
/* Struct for command definitions - it stores commands requirement, processing function...
 * When adding new processing functions just add it to commands definition and add it's processing function. */
typedef struct {
    char *name;             // command name
    int command_category;   // type of command (table edit/row selection/data processing)
    function_ptr processing_function;
}Command_t;



bool string_compare(char *first, char *second)
{
    return strcmp(first, second) == 0;
}

// Function checks if provided command name is valid command name
bool is_command(char **provided_command_name, char *command_name)
{
    char *found_at = strstr(*provided_command_name, command_name);
    if (found_at != *provided_command_name)
        return false;
    (*provided_command_name) += strlen(command_name);   // used to move pointer behind command name, to arguments
    return true;
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
    if (opened_file)
        return opened_file;
    print_error("Could not open file!");
    return NULL;
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

void initialize_cell_pointers(Table *table, table_index row, table_index columns, table_index starting_column)
{
    for (; starting_column < columns; starting_column++)
        table->rows[row]->cells[starting_column] = NULL;
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

        initialize_cell_pointers(table, row_index, dimensions.columns, 0);  // initializing with NULL, it will be overwrited in cell loading
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
    int character_before = 0;       // at the first iteration it wont be initialized

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

void destruct_user_variables(char **variables)
{
    for (short var_index = 0; var_index < USER_VARIABLES_COUNT; var_index++)
        if (variables[var_index])
            free(variables[var_index]);

    free(variables);
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
    size_t remaining_commands_size = strlen(*cell_start);
    if (*cell_start[0] == ';' || *cell_start[0] == '\n')
        (*cell_start)++;

    char *command_end = *cell_start;
    bool inside_quotation = false;
    if (!remaining_commands_size)
        return NULL;

    for (unsigned int position = 0; position < remaining_commands_size; position++)
    {
        if ((*cell_start)[position] == '\"' && ((!first_command || position) && (*cell_start)[position - 1] != '\\'))
            inside_quotation = inside_quotation ? false : true;

        if ((!inside_quotation && (*cell_start)[position] == ';') || (*cell_start)[position] == '\n')
            break;

        if (command_end[0] != '\0')
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

void set_cell_directions(CellsSelector *selector, table_index row, table_index column)
{
    selector->starting_row = row;
    selector->starting_cell = column;
    selector->ending_row = row;
    selector->ending_cell = column;
}

unsigned short process_min_max_selectors(CellsSelector *selector, bool min, Table *table)
{
    CellsSelector selected = {0};
    bool found = false;
    float selected_value = min ? FLT_MAX : FLT_MIN;

    for (table_index row = selector->starting_row; row < selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column < selector->ending_cell; column++)
        {
            float cell_value;
            bool converted = get_numeric_cell_value(table->rows[row]->cells[column], &cell_value);
            if (!converted)
                continue;

            if ((min && cell_value < selected_value) || (!min && cell_value > selected_value))
            {
                set_cell_directions(&selected, row, column);
                selected_value = cell_value;
                found = true;
            }
        }

    if (found)
        set_cell_directions(selector, selected.starting_row, selected.starting_cell);

    return EXIT_SUCCESS;
}

unsigned short process_find_selector(CellsSelector *selector, const char *command, Table *table)
{
    char *selector_ending = strchr(command, ']');
    unsigned short selector_length = selector_ending - command + 1;     // +1 because selector ending is not included
    if (!selector_ending || (selector_length - 6) <= 1)                 // selector length could not be 1, it s starting at 1
    {
        print_error("Invalid selecor!");
        return EXIT_FAILURE;
    }

    *selector_ending = '\0';    // marking selector ending as end of command, it will be used for searching in cells
    command += 6;               // string starts at 6. position
    bool found = false;

    for (table_index row = selector->starting_row; row < selector->ending_row && !found; row++)
        for (table_index column = selector->starting_cell; column < selector->ending_cell; column++)
        {
            char *contains = strstr(table->rows[row]->cells[column], command);
            if (contains)
            {
                set_cell_directions(selector, row, column);
                found = true;
                break;
            }
        }
    return selector_length;
}

unsigned short swap_selectors(CellsSelector *destination, CellsSelector *source)
{
    destination->starting_row  = source->starting_row;
    destination->starting_cell = source->starting_cell;
    destination->ending_row    = source->ending_row;
    destination->ending_cell   = source->ending_cell;

    return EXIT_SUCCESS;
}

// Function process special selectors ([min], [max], [find STR], [_])
unsigned short process_special_selectors(CellsSelector *selector, const char *command, Table *table, CellsSelector *temporary_selector)
{
    if (strstr(command, "[min]") == command)
        return process_min_max_selectors(selector, true, table);
    else if (strstr(command, "[max]") == command)
        return process_min_max_selectors(selector, false, table);
    else if (strstr(command, "[find") == command)
        return process_find_selector(selector, command, table);
    else if (strstr(command, "[_]") == command)
        return swap_selectors(selector, temporary_selector);

    return EXIT_SUCCESS;
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
        table_index value;
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
    if (selector_index < 3)
    {
        selector->ending_row = selector->starting_row;
        selector->ending_cell = selector->starting_cell;
    }
    return EXIT_SUCCESS;
}

// Function returns selector length. Minimal length of selector is 3 characters - [_]. So 0-3 can be used as error codes.
unsigned short process_selector(CellsSelector *selector, char *command, Table *table, CellsSelector *temporary_selector)
{
    if (command[0] != '[' || (strlen(command) > 2 && command[1] == 's'))      // first character is not a selector, skip [set] selector
        return EXIT_SUCCESS;

    unsigned short result;

    result = process_special_selectors(selector, command, table, temporary_selector);
    if (result == EXIT_FAILURE)
        return EXIT_FAILURE;

    if (!result)
    {
        result = process_normal_selector(selector, command);
        if (result == EXIT_FAILURE)
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void rollback_rows(Table *table, TableSize *size, TableSize *resize_to)
{
    for (table_index row = size->rows; row < resize_to->rows; row++)
    {
        for (table_index column = 0; column < size->columns; column++)
            free(table->rows[row]->cells[column]);
        free(table->rows[row]);
    }
}

unsigned short add_rows(Table *table, TableSize *size, TableSize *resize_to)
{
    TableRow  **rows = (TableRow **)realloc(table->rows, sizeof(TableRow *) * resize_to->rows);
    if (!rows)
        return EXIT_FAILURE;
    table->rows = rows;

    bool rollback = false;
    for (table_index row = size->rows; row < resize_to->rows && !rollback; row++)
    {
        char **row_cells = (char **)malloc(sizeof(char *) * size->columns);
        if (!row_cells)
        {
            rollback = true;
            break;
        }
        table->rows[row]->cells = row_cells;

        initialize_cell_pointers(table, row, size->columns, 0);

        int filling_result = fill_with_empty_cells(table, *size, row, -1);    // -1 as column because that funcion have +1 to that value

        if (filling_result == 1)
        {
            rollback = true;
            break;
        }
    }
    if (rollback)
    {
        rollback_rows(table, size, resize_to);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void rollback_cells(Table *table, TableSize *size, TableSize *resize_to)
{
    for (table_index row = 0; row < resize_to->rows; row++)
        for (table_index column = size->columns; column < resize_to->columns; column++)
            free(table->rows[row]->cells[column]);

}

unsigned short add_columns(Table *table, TableSize *size, TableSize *resize_to)
{
    bool rollback = false;
    table_index total_rows = resize_to->rows ? resize_to->rows : size->rows;

    for (table_index row = 0; row < total_rows && !rollback; row++)
    {
        char **row_cells = (char **)realloc(table->rows[row]->cells, sizeof(char *) * resize_to->columns);
        if (!row_cells)
        {
            rollback = true;
            break;
        }
        table->rows[row]->cells = row_cells;

        initialize_cell_pointers(table, row, resize_to->columns, size->columns);
        if (fill_with_empty_cells(table, *resize_to, row, size->columns - 1))
        {
            rollback = true;
            break;
        }
    }

    if (rollback)
    {
        rollback_cells(table, size, resize_to);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

unsigned short resize_table(Table *table, TableSize *size, TableSize *resize_to)
{
    if (resize_to->rows != size->rows && add_rows(table, size, resize_to))
    {
        print_error("Could not allocate memory for new row(s)!");
        return EXIT_FAILURE;
    }

    if (resize_to->columns != size->columns && add_columns(table, size, resize_to))
    {
        print_error("Could not allocate memory for new column(s)!");
        return EXIT_FAILURE;
    }
    size->rows = resize_to->rows;
    size->columns = resize_to->columns;

    return EXIT_SUCCESS;
}

// Function will resize table, if necessary. There is lot of -1 because of different indexing
// size is indexed from 1 and selector from 0.
unsigned short resize_table_if_necessary(Table *table, TableSize *size, CellsSelector *selector)
{
    if ((size->rows - 1) >= selector->ending_row && (size->columns - 1) >= selector->ending_cell)   // -1 bcs size is indexed from 1
        return EXIT_SUCCESS;

    TableSize resize_to = {size->rows, size->columns};
    if ((size->rows - 1) < selector->ending_row)
        resize_to.rows = selector->ending_row + 1;      // +1 because of difference in indexing and "<" operator in loops
    if ((size->columns - 1) < selector->ending_cell)
        resize_to.columns = selector->ending_cell + 1;  // same
    return resize_table(table, size, &resize_to);
}

unsigned short set(Table *table, char *to_set, CellsSelector *selector)
{
    size_t to_set_length = strlen(to_set);

    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            char *resized_cell = (char *)realloc(table->rows[row]->cells[column], sizeof(char) * to_set_length + 1);
            if (!resized_cell)
            {
                if (to_set_length > strlen(table->rows[row]->cells[column]))
                {
                    print_error("Could not allocate memory for cell!");
                    return EXIT_FAILURE;
                }
                resized_cell = table->rows[row]->cells[column];
            }
            copy_to_array(resized_cell, to_set, to_set_length);
            table->rows[row]->cells[column] = resized_cell;
        }
    return EXIT_SUCCESS;
}

unsigned short clear(Table *table, CellsSelector *selector)
{
    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            char *resized_cell = (char *)realloc(table->rows[row]->cells[column], sizeof(bool));
            if (!resized_cell)      // in case realloc failed, dont need to end, original cell will be used
            {
                resized_cell = table->rows[row]->cells[column];
            }
            copy_to_array(resized_cell, "", 0);
            table->rows[row]->cells[column] = resized_cell;
        }
    return EXIT_SUCCESS;
}

unsigned short swap(Table *table, char *command, CellsSelector *selector)
{
    CellsSelector swap_with = {0};
    process_normal_selector(&swap_with, command);

    char *temp = table->rows[selector->starting_row]->cells[selector->ending_row];
    table->rows[selector->starting_row]->cells[selector->ending_row] = table->rows[swap_with.starting_row]->cells[swap_with.starting_cell];
    table->rows[swap_with.starting_row]->cells[swap_with.starting_cell] = temp;

    return EXIT_SUCCESS;
}

// Function will set float value to char array
unsigned short set_numeric_value_to_cell(Table *table, CellsSelector *selector, float value)
{
    int no_of_digits = snprintf(NULL, 0, "%g", value);
    char result[no_of_digits];
    result[no_of_digits] = '\0';
    sprintf(result,"%g", value);

    return set(table, result, selector);
}

// Function for sum/avg/count
unsigned short cell_counting_commands(Table *table, char *command, CellsSelector *selector, short what_to_do)
{
    CellsSelector save_to = {0};
    process_normal_selector(&save_to, command);
    float sum = 0;
    table_index values_count = 0;

    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            float cell_value;
            bool valid = get_numeric_cell_value(table->rows[row]->cells[column], &cell_value);
            if (!valid && what_to_do != 2)
                continue;
            sum += cell_value;
            values_count++;
        }

    if (what_to_do == 0)
        return set_numeric_value_to_cell(table, &save_to, sum);
    else if (what_to_do == 1)
        return set_numeric_value_to_cell(table, &save_to, (sum / (values_count ? (float)values_count : 1)));    // prevent to divide by 0
    else
        return set_numeric_value_to_cell(table, &save_to, (float)values_count);
}

unsigned short sum(Table *table, char *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 0);
}

unsigned short avg(Table *table, char *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 1);
}

unsigned short count(Table *table, char *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 2);
}

unsigned short len(Table *table, char *command, CellsSelector *selector)
{
    CellsSelector save_to = {0};
    process_normal_selector(&save_to, command);

    size_t cell_length = strlen(table->rows[selector->starting_row]->cells[selector->starting_cell]);
    set_numeric_value_to_cell(table, &save_to, (float)cell_length);
    return EXIT_SUCCESS;
}

char **get_variable_pointer(char *variable_index, char **variables)
{
    float index;
    bool valid = get_numeric_cell_value(variable_index, &index);
    if (!valid && index < USER_VARIABLES_COUNT)     // user have N variables but indexed from 0
    {
        print_error("Invalid variable index!");
        return NULL;
    }
    return &variables[(unsigned short)index];
}

unsigned short def(Table *table, char **variable, CellsSelector *selector)
{
    char *cell = table->rows[selector->starting_row]->cells[selector->starting_cell];
    size_t cell_length = strlen(cell);

    char *copy = (char *)malloc(sizeof(char) * cell_length + 1);
    if (!copy)
    {
        print_error("Could not allocate memory for cell copy!");
        return EXIT_FAILURE;
    }
    copy_to_array(copy, cell, cell_length);
    *variable = copy;
    printf("%s", copy);
    return EXIT_SUCCESS;
}

unsigned short use(Table *table, char **variable, CellsSelector *selector)
{
    return set(table, *variable, selector);
}

unsigned short inc(char **variable)
{
    float value;
    bool is_number = get_numeric_cell_value(*variable, &value);

    value = is_number ? (value + 1) : 1;

    int no_of_digits = snprintf(NULL, 0, "%g", value);
    char *resized = (char *)realloc(*variable, no_of_digits + 1);
    if (!resized && (size_t)no_of_digits > strlen(*variable))
    {
        print_error("Could not allocate enough space for variable!");
        return EXIT_FAILURE;
    }else
        *variable = resized;

    sprintf(resized,"%g", value);
    return EXIT_SUCCESS;
}

char *get_variable_index_position(char *command)
{
    char *found = strchr(command, '_');
    if (!found || ((found + 1)[0] == '\0'))
    {
        print_error("Invalid syntax!");
        return NULL;
    }
    return found + 1;   // move pointer behind _
}

unsigned short process_temporary_selectors(Table *table, char *command, CellsSelector *selector, char **user_variables)
{
    char *variable_index = get_variable_index_position(command);
    if (!variable_index)
    {
        if (strstr(command, "def ") || strstr(command, "use ") || strstr(command, "inc "))
            return EXIT_FAILURE;
        return EXIT_SUCCESS;    // command is not selector
    }

    char **variable = get_variable_pointer(command, user_variables);
    if (!variable ||                                   // checking variable validity
        (!*variable && !strstr(command, "def")))  //checking if variable is initialized for "use" and "inc" commands
        return EXIT_FAILURE;

    unsigned short result = EXIT_SUCCESS;
    if (is_command(&command, "def _"))
        result = def(table, variable, selector);
    else if (is_command(&command, "use _"))
        result = use(table, variable, selector);
    else if (is_command(&command, "inc _"))
        result = inc(variable);

    return result;
}

// Funcion checks if selected range is just one cell, or it s range of cells.
bool is_range(CellsSelector *selector)
{
    if (selector->starting_row != selector->ending_row || selector->starting_cell != selector->ending_cell)
        return true;
    return false;
}

unsigned short process_command(Table *table, TableSize size, char *command, CellsSelector *selector, CellsSelector *temp_selector, char **user_variables)
{
    (void)size;
    unsigned short return_code = 0;

    if (is_command(&command, "set "))
        return_code = set(table, command, selector);
    else if (is_command(&command, "clear"))
        return_code = clear(table, selector);
    else if (is_command(&command, "[set]"))
        return_code = swap_selectors(temp_selector, selector);
    else if (is_command(&command, "swap "))
    {
        if (!is_range(selector))
            swap(table, command, selector);
        else
        {
            print_error("Range is not one cell!");
            return_code = EXIT_FAILURE;
        }
    }else if (is_command(&command, "sum "))
        return_code = sum(table, command, selector);
    else if (is_command(&command, "avg "))
        return_code = avg(table, command, selector);
    else if (is_command(&command, "count "))
        return_code = count(table, command, selector);
    else if (is_command(&command, "len "))
    {
        if (!is_range(selector))
            len(table, command, selector);
        else
        {
            print_error("Range is not one cell!");
            return_code = EXIT_FAILURE;
        }
    }else
        return_code = process_temporary_selectors(table, command, selector, user_variables);

    return return_code;
}

int parse_commands(Table *table, TableSize *size, int arg_count, char **arguments, char **user_variables)
{
    char *command_start = arguments[arg_count - 2];
    char *command_end = command_start;
    CellsSelector selected = {1, 1, 1, 1};
    CellsSelector users_saved_selector = {0, 0, 0, 0};

    bool first_command = true;
    unsigned short command_length = 0;

    while ((command_end = get_command_from_argument(&command_end, first_command, &command_length)) != NULL)
    {
        if (command_length > MAXIMUM_COMMAND_LENGTH)
            return EXIT_FAILURE;

        char command[command_length + 1];
        copy_to_array(command, command_start, command_length);

        unsigned short selector_length = process_selector(&selected, command, table, &users_saved_selector);
        unsigned short resizing_failed = resize_table_if_necessary(table, size, &selected);
        if (selector_length == EXIT_FAILURE || resizing_failed)
            return EXIT_FAILURE;

        if (process_command(table, *size, command_end, &selected, &users_saved_selector, user_variables))
            return EXIT_FAILURE;

        printf("%lld, %lld, %lld, %lld\n", selected.starting_row, selected.starting_cell, selected.ending_row, selected.ending_cell);

        first_command = false;
        command_start = command_end;
        command_length = 0;
    }
    return EXIT_SUCCESS;
}

// Function initializes array for user variables. Using char because it could be string.
char ** initialize_user_variables()
{
    char **variables = (char **)malloc(sizeof(char *) * USER_VARIABLES_COUNT);   // user could define 10 variables
    if (!variables)
    {
        print_error("Could not allocate memory for user variables!");
        return NULL;
    }
    for (short var_index = 0; var_index < USER_VARIABLES_COUNT; var_index++)
        variables[var_index] = NULL;
    return variables;
}

void print_variables(char **variables)
{
    printf("==USED VARIABLES==\n");
    for (int i = 0; i < USER_VARIABLES_COUNT; i++)
        if (variables[i])
            printf("%d. : '%s'\n", i, variables[i]);
    printf("==VARIABLES==\n");
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

    char **user_variables = initialize_user_variables();

    if (!successful_initialization || !user_variables)
    {
        int successfully_loaded = load_table(table_file, table, delimiter, size);
        if (!successfully_loaded)
        {
            if (!parse_commands(table, &size, arg_count, arguments, user_variables))
            {
                print_table(table, size);
                save_table(table, table_file, size, delimiter[0]);
            }
        }
    }
    print_variables(user_variables);

    destruct_table(table, size);
    destruct_user_variables(user_variables);
    fclose(table_file);
    return EXIT_SUCCESS;
}
