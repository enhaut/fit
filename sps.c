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

void get_table_size(FILE *table_file, char *delimiters, TableSize *size)
{
    table_index rows = 0;
    table_index columns = 0;
    table_index row_columns = 0;
    int loaded_character;
    bool inside_quotation = false;  // used to prevent counting delimiters inside " " block

    while ((loaded_character = getc(table_file)) != EOF)
    {
        if (loaded_character == '\"') {
            inside_quotation = inside_quotation ? false : true;
            continue;
        }

        if (!inside_quotation && is_character_delimiter(delimiters, loaded_character))
            row_columns++;

        if (loaded_character == '\n') {
            rows++;
            if (row_columns > columns)
                columns = row_columns;  // save the count of cells in the biggest row
            row_columns = 0;            // new row reached, reset counter
        }
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
    if (!dimensions.rows || !dimensions.columns) {
        *result = EXIT_FAILURE;
        return NULL;
    }

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

void remove_special_characters(char *cell, table_index cell_size)
{

    char *found_backslash;
    while ((found_backslash = strchr(cell, '\\')) != NULL)
    {
        if (found_backslash && found_backslash[1] != '"') {
            memmove(found_backslash, &found_backslash[1], sizeof(char) * (strlen(found_backslash) - 1));
            cell[(cell_size--) - 1] = '\0';
        }
    }

    if (cell[0] == '"' && cell[cell_size - 1]  == '"')
    {
        memmove(&cell[0], &cell[1], sizeof(char) * (cell_size - 2));
        cell[cell_size - 2] = '\0';
    }

}

char * load_table_cell(FILE *table_file, char *delimiters, bool *last_cell)
{
    table_index cell_length = INITIAL_CELL_SIZE;
    char *cell = (char *) malloc(sizeof(char) * cell_length + 1);
    if (!cell)
        return NULL;
    for (table_index x = 0; x < cell_length; x++)
        cell[x] = 0;

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

    remove_special_characters(cell, position);
    dealloc_unused_cell_part(&cell, position);

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

void destruct_commands(Command_t *commands, unsigned short count)
{
    for (unsigned short i  = 0; i < count; i++)
        free(commands[i].name);

    free(commands);
}

/* Function will return size of savable table. That will remove empty columns at the end of table. */
TableSize get_savable_table_size(Table *table, TableSize size)
{
    TableSize savable_size;
    savable_size.columns = size.columns;
    savable_size.rows = size.rows;
    if (!savable_size.columns || !savable_size.rows)
        return savable_size;

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
            if (!table->rows[row]->cells[column])
                continue;
            printf("%s", table->rows[row]->cells[column]);
            if (column < (size.columns - 1))
                printf(",");
        }
        printf("\n");
    }
}

bool contains_delimiter(char *cell, char *delimiters)
{
    size_t cell_len = strlen(cell);
    bool contains = false;
    for (table_index position = 0; position < cell_len && !contains; position++)
        contains = is_character_delimiter(delimiters, cell[position]);

    return contains;
}

void save_table(Table *table, FILE *table_file, TableSize size, char *delimiters)
{
    freopen(NULL, "w", table_file);     // file is opened in r mode, so reopen it to rewrite content
    size = get_savable_table_size(table, size);

    for (table_index row = 0; row < size.rows; row++)
    {
        for (table_index column = 0; column < size.columns; column++)
        {
            bool should_be_quoted = contains_delimiter(table->rows[row]->cells[column], delimiters);
            char quot[2] = "\"";
            if (!should_be_quoted)
                quot[0] = '\0';

            fprintf(table_file, "%s%s%s", quot, table->rows[row]->cells[column], quot);

            if (column < (size.columns - 1))    // write delimiter behind not last columns
                fputc(delimiters[0], table_file);
        }
        fputc('\n', table_file);
    }
}

void print_commands(Command_t *command, unsigned short count)
{
    printf("===COMMANDS: %d===\n", count);
    for (unsigned short i = 0; i < count; i++) {
        printf("%d.: command:\t%s, type:\t%d\n", i, command[i].name, command[i].command_category);
    }
    printf("===COMMANDS===\n");
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

unsigned short process_min_max_selectors(CellsSelector *selector, Table *table, Command_t *command)
{
    CellsSelector selected = {0};
    bool found = false;
    bool min = string_compare(command->name, "[min]");
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

unsigned short process_find_selector(CellsSelector *selector, Command_t *command, Table *table)
{
    char *selector_ending = strchr(command->name, ']');
    unsigned short selector_length = selector_ending - command->name + 1;     // +1 because selector ending is not included
    if (!selector_ending || (selector_length - 6) <= 1)                 // selector length could not be 1, it s starting at 1
    {
        print_error("Invalid selecor!");
        return EXIT_FAILURE;
    }

    *selector_ending = '\0';    // marking selector ending as end of command, it will be used for searching in cells
    command->name += 6;               // string starts at 6. position
    bool found = false;

    for (table_index row = selector->starting_row; row < selector->ending_row && !found; row++)
        for (table_index column = selector->starting_cell; column < selector->ending_cell; column++)
        {
            char *contains = strstr(table->rows[row]->cells[column], command->name);
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
unsigned short process_special_selectors(CellsSelector *selector, Command_t *command, Table *table, CellsSelector *temporary_selector)
{
    if (string_compare(command->name, "[_]"))
        return command->processing_function(selector, temporary_selector);
    else if (string_compare(command->name, "[set]"))
        return command->processing_function(temporary_selector, selector);
    else
        return command->processing_function(selector, table, command);

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

void set_to_selector(CellsSelector *selector, table_index s_row, table_index s_cell, table_index e_row, table_index e_cell)
{
    selector->starting_row = s_row;
    selector->starting_cell = s_cell;
    selector->ending_row = e_row;
    selector->ending_cell = e_cell;
}

unsigned short process_relative_selector(CellsSelector *selector, TableSize size, char selectors[][21])
{
    bool first_is_comma = selectors[0][0] == '_';
    bool second_is_comma = selectors[1][0] == '_';
    float value;
    bool valid = true;

    if (first_is_comma && second_is_comma)
        set_to_selector(selector, 0, 0, size.rows - 1, size.columns - 1);
    else if ((first_is_comma && !second_is_comma) || (!first_is_comma && second_is_comma)) {
        valid = get_numeric_cell_value(selectors[first_is_comma ? 1 : 0], &value);
        if (!valid)
        {
            print_error("Invalid selector!XOXOXOXOXO");
            return EXIT_FAILURE;
        }

        value -= 1;     // adjust indexes
        if (first_is_comma)
            set_to_selector(selector, 0, (table_index)value, size.rows - 1, (table_index)value);
        else
            set_to_selector(selector, (table_index)value, 0, (table_index)value, size.rows - 1);
    }

    return EXIT_SUCCESS;
}

unsigned short process_normal_selector(CellsSelector *selector, Command_t *command, TableSize size)
{
    char *selector_values = command->name;
    selector_values[strlen(selector_values) - 1] = '\0';    // remove trailing ]
    selector_values = strtok((strchr(selector_values, '[') + 1), SELECTION_COMMANDS_DELIMITER);

    char selectors[4][21] = {0};      // 4 for 4 selectors and 21 for up to 20 digits of each selector (ull could story up to 20 digits long number)
    short selector_index = -1;

    while (selector_values != NULL && ++selector_index < 4)     // 4 is maximum # of selectors
    {
        copy_to_array(selectors[selector_index], selector_values, strlen(selector_values));
        selector_values = strtok(NULL, SELECTION_COMMANDS_DELIMITER);
    }

    if (selectors[0][0] == '_' || selectors[1][0] == '_')
        return process_relative_selector(selector, size, selectors);

    bool valid = true;
    float value;
    for (selector_index = 0; selector_index < 4; selector_index++)  // 4 is maximum # of selectors
    {
        short offset = 0;
        if (selector_index >= 2 && selectors[selector_index][0] == '\0')
            offset = -2;

        valid = get_numeric_cell_value(selectors[selector_index + offset], &value);

        if (selectors[selector_index][0] == '-')
        {
            valid = true;
            value = (float) (selector_index == 2 ? size.rows : size.columns);
        }

        if (!valid)
        {
            print_error("Invalid selector!");
            return EXIT_FAILURE;
        }
        *get_selector_from_index(selector_index, selector) = (table_index)value - 1;
    }

    return EXIT_SUCCESS;
}

// Function returns selector length. Minimal length of selector is 3 characters - [_]. So 0-3 can be used as error codes.
unsigned short process_selector(CellsSelector *selector, Command_t *command, Table *table, CellsSelector *temporary_selector, TableSize size)
{
    if ((command->command_category == 3 && process_special_selectors(selector, command, table, temporary_selector)) ||
        (command->command_category == 4 && process_normal_selector(selector, command, size)))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

// Funcion checks if selected range is just one cell, or it s range of cells.
bool is_range(CellsSelector *selector)
{
    if (selector->starting_row != selector->ending_row || selector->starting_cell != selector->ending_cell)
        return true;
    return false;
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
        TableRow *new_row = (TableRow *)malloc(sizeof(TableRow));
        char **row_cells = (char **)malloc(sizeof(char *) * size->columns);
        if (!new_row || !row_cells)
        {
            rollback = true;
            break;
        }
        table->rows[row] = new_row;
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

unsigned short set_to_cell(char **cell, char *to_set, size_t to_set_length)
{
    char *resized_cell = (char *)realloc(*cell, sizeof(char) * to_set_length + 1);
    if (!resized_cell)
    {
        if (to_set_length > strlen(*cell))
        {
            print_error("Could not allocate memory for cell!");
            return EXIT_FAILURE;
        }
        resized_cell = *cell;
    }
    copy_to_array(resized_cell, to_set, to_set_length);
    *cell = resized_cell;

    return EXIT_SUCCESS;
}

unsigned short set(Table *table, Command_t *command, CellsSelector *selector)
{
    char *to_set = command->name + 4;
    remove_special_characters(to_set, strlen(to_set));
    size_t to_set_length = strlen(to_set);

    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            if (set_to_cell(&table->rows[row]->cells[column], to_set, to_set_length))
                return EXIT_FAILURE;
        }
    return EXIT_SUCCESS;
}

unsigned short clear(Table *table, Command_t *command, CellsSelector *selector)
{
    (void)command;
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

unsigned short swap(Table *table, Command_t *command, CellsSelector *selector)
{
    CellsSelector swap_with = {0};
    TableSize size = {0};
    process_normal_selector(&swap_with, command, size);

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

    return set_to_cell(&table->rows[selector->starting_row]->cells[selector->starting_cell], result, no_of_digits);
}

// Function for sum/avg/count
unsigned short cell_counting_commands(Table *table, Command_t *command, CellsSelector *selector, short what_to_do)
{
    CellsSelector save_to = {0};
    TableSize size = {0};
    process_normal_selector(&save_to, command, size);

    float sum = 0;
    table_index values_count = 0;

    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            float cell_value;
            bool valid = get_numeric_cell_value(table->rows[row]->cells[column], &cell_value);
            if (!valid || !strlen(table->rows[row]->cells[column]))
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

unsigned short sum(Table *table, Command_t *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 0);
}

unsigned short avg(Table *table, Command_t *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 1);
}

unsigned short count(Table *table, Command_t *command, CellsSelector *selector)
{
    return cell_counting_commands(table, command, selector, 2);
}

unsigned short len(Table *table, Command_t *command, CellsSelector *selector)
{
    CellsSelector save_to = {0};
    TableSize size = {0};
    process_normal_selector(&save_to, command, size);

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
    return EXIT_SUCCESS;
}

unsigned short use(Table *table, char **variable, CellsSelector *selector)
{
    return set_to_cell(&table->rows[selector->starting_row]->cells[selector->starting_cell], *variable, strlen(*variable));
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

unsigned short process_temporary_selectors(Table *table, Command_t *command, CellsSelector *selector, char **user_variables)
{
    char *variable_index = get_variable_index_position(command->name);
    char **variable = get_variable_pointer(variable_index, user_variables);

    if (!variable ||                                            // checking variable validity
        (!*variable && command->processing_function != def))    //checking if variable is initialized for "use" and "inc" commands
    {
        print_error("Variable is not defined!");
        return EXIT_FAILURE;
    }

    if (command->processing_function == def)
        return command->processing_function(table, variable, selector);

    for (table_index row = selector->starting_row; row <= selector->ending_row; row++)
        for (table_index column = selector->starting_cell; column <= selector->ending_cell; column++)
        {
            unsigned short return_code;
            if (command->processing_function == inc)
                return_code = inc(variable);
            else
                return_code = command->processing_function(table, variable, selector);

            if(return_code)
                return return_code;
        }

    return EXIT_SUCCESS;
}

unsigned short irow_arow(Table *table, TableSize *size, CellsSelector *selector, Command_t *command)
{
    TableSize resize_to = {size->rows + 1, size->columns};
    if (resize_table(table, size, &resize_to))
        return EXIT_FAILURE;

    TableRow *new_row = table->rows[resize_to.rows - 1]; // size is indexed from 1 so -1
    table_index destination_index = string_compare(command->name, "irow") ? selector->starting_row : (selector->ending_row + 1);
    table_index to_move = resize_to.rows - destination_index - 1;

    memmove(&(table->rows[destination_index + 1]), &(table->rows[destination_index]), sizeof(TableRow *) * to_move);
    table->rows[destination_index] = new_row;

    return EXIT_SUCCESS;
}

unsigned short icol_acol(Table *table, TableSize *size, CellsSelector *selector, Command_t *command)
{
    TableSize resize_to = {size->rows, size->columns + 1};
    if (resize_table(table, size, &resize_to))
        return EXIT_FAILURE;

    bool acol = string_compare(command->name, "acol");

    for (table_index row = 0; row < size->rows; row++)
    {
        char *new_cell = table->rows[row]->cells[resize_to.columns - 1];
        table_index destination_index = acol ? (selector->ending_cell + 1) : selector->starting_cell;
        table_index to_move = resize_to.columns - destination_index - 1;
        memmove(&(table->rows[row]->cells[destination_index + 1]), &(table->rows[row]->cells[destination_index]), sizeof(char *) * to_move);
        table->rows[row]->cells[destination_index] = new_cell;
    }

    return EXIT_SUCCESS;
}

unsigned short process_table_struct_commands(Table *table, TableSize *size, CellsSelector *selector, Command_t *command)
{
    return command->processing_function(table, size, selector, command);
}

unsigned short process_command(Table *table, Command_t *command, CellsSelector *selector)
{
    if (command->processing_function == len || command->processing_function == swap)
        if (is_range(selector))
        {
            print_error("Functions \"len\" and \"swap\" does not support range of cells!");
            return EXIT_FAILURE;
        }
    return command->processing_function(table, command, selector);
}

int parse_commands(Table *table, TableSize *size, Command_t *commands, unsigned short count, char **user_variables)
{
    CellsSelector selected              = {0, 0, 0, 0};
    CellsSelector users_saved_selector  = {0, 0, 0, 0};

    for (unsigned short command_index = 0; command_index < count; command_index++)
    {
        if (commands[command_index].command_category == 3 || commands[command_index].command_category == 4)
            if (process_selector(&selected, &commands[command_index], table, &users_saved_selector, *size))
                return EXIT_FAILURE;

        unsigned short command_category =  commands[command_index].command_category;

        if (resize_table_if_necessary(table, size, &selected) ||
            (command_category == 1 && process_command(table, &commands[command_index], &selected)) ||
            (command_category == 5 && process_temporary_selectors(table, &commands[command_index], &selected, user_variables)) ||
            (command_category == 6 && process_table_struct_commands(table, size, &selected, &commands[command_index])))
            return EXIT_FAILURE;

        printf("%lld, %lld, %lld, %lld\n", selected.starting_row, selected.starting_cell, selected.ending_row, selected.ending_cell);
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

void copy_command_definitions(Command_t *destination_array)
{
    Command_t commands[SUPPORTED_COMMANDS_COUNT] = {
            {"set",     1, set},
            {"clear",   1, clear},
            {"swap",   1, swap},
            {"sum",     1, sum},
            {"avg",     1, avg},
            {"count",   1, count},
            {"len",     1, len},
            {"[min]",   3, process_min_max_selectors},
            {"[max]",   3, process_min_max_selectors},
            {"[_]",     3, swap_selectors},
            {"[find ",  3,        process_find_selector},
            {"[set]",   3,        swap_selectors},
            {"def",   5,          def},
            {"use",   5,          use},
            {"inc",   5,          inc},
            {"irow", 6,           irow_arow},
            {"arow", 6,           irow_arow},
            {"drow", 6,           NULL},
            {"icol", 6, icol_acol},
            {"acol", 6, icol_acol},
            {"dcol", 6, NULL},
            {"SLCTRS",  4,        process_selector},   // selectors have to be last
            //{"def", 2, def},
            //{"len", 1, len},
    };
    for (unsigned short command_index = 0; command_index < SUPPORTED_COMMANDS_COUNT; command_index++)
        destination_array[command_index] = commands[command_index];
}

unsigned short copy_commands_file_content(FILE *file, char **commands, unsigned int array_capacity)
{
    int loaded_character;
    long position = 0;
    while ((loaded_character = getc(file)) != EOF)
    {
        if (position == array_capacity)
        {
            array_capacity *= 2;
            char *resized = (char *)realloc(*commands, sizeof(char) * array_capacity + 1);
            if (!resized)
            {
                print_error("Could not allocate enough memory for commands!");
                return EXIT_FAILURE;
            }
            *commands = resized;
        }
        if (loaded_character == '\n')// replace \n commands delimiter with correct delimiter
        {
            int next_char = getc(file);
            loaded_character = '\0';
            if (next_char != EOF)
                loaded_character = ';';
            ungetc(next_char, file);
        }
        (*commands)[position] = (char) loaded_character;
        position++;
    }
    (*commands)[position] = '\0';

    return EXIT_SUCCESS;
}

char *get_commands_from_file(char *filename)
{
    FILE *commands_file = file_loader(filename, "r");
    if (!commands_file)
        return NULL;

    unsigned int array_capacity = 50;
    char *file_commands = (char *)malloc(sizeof(char) * array_capacity + 1);
    if (!file_commands)
    {
        print_error("Could not allocate memory for commands!");
        return NULL;
    }

    if (copy_commands_file_content(commands_file, &file_commands, array_capacity))
        return NULL;

    size_t commands_length = strlen(file_commands);
    dealloc_unused_cell_part(&file_commands, commands_length);

    fclose(commands_file);
    return file_commands;
}

Command_t *allocate_commands_array()
{
    Command_t *commands = (Command_t *)malloc(sizeof(bool));    // it is reallocated on-demand, so saving memory for now
    if (!commands)
    {
        print_error("Could not allocate memory for commands!");
        return NULL;
    }
    return commands;
}

unsigned short get_command_def_index(Command_t *command_definitions, char *command_start)
{
    unsigned short command_def_index = SUPPORTED_COMMANDS_COUNT + 1;    // initializing with value behind valid range

    for (unsigned short command_index = 0; command_index < (SUPPORTED_COMMANDS_COUNT - 1) && command_def_index > SUPPORTED_COMMANDS_COUNT; command_index++)
    {
        if (strstr(command_start, command_definitions[command_index].name) == command_start)
            command_def_index = command_index;
    }

    if (command_def_index > SUPPORTED_COMMANDS_COUNT && command_start[0] == '[' && command_start[1] != 's')   // selectors
        command_def_index = SUPPORTED_COMMANDS_COUNT-1;

    return command_def_index;
}

Command_t get_command_from_arguments(char *command_start, unsigned short command_length, Command_t command_def)
{
    char *command_content = (char *)malloc(sizeof(char) * command_length + 1);
    Command_t command = {0};
    if (!command_content)
    {
        print_error("Could not allocate memory for command!");
        return command;
    }
    copy_to_array(command_content, command_start, command_length);
    command.name = command_content;
    command.command_category = command_def.command_category;
    command.processing_function = command_def.processing_function;
    return command;
}

short save_commands(char *commands_source, Command_t *command_definitions, Command_t **commands)
{
    short commands_array_size = 0;
    char *command_start = commands_source;
    char *command_end = commands_source;

    bool first_command = true;
    unsigned short command_length = 0;

    while ((command_end = get_command_from_argument(&command_end, first_command, &command_length)) != NULL)
    {
        if (command_length > MAXIMUM_COMMAND_LENGTH)
            return commands_array_size;

        unsigned short command_def_index = get_command_def_index(command_definitions, command_start);
        if (command_def_index > SUPPORTED_COMMANDS_COUNT)
            return commands_array_size;

        Command_t *reallocated_commands = realloc(*commands, sizeof(Command_t) * (++commands_array_size));
        if (!reallocated_commands)
        {
            print_error("Could not reallocate commands array!");
            return commands_array_size;
        }
        *commands = reallocated_commands;

        Command_t allocated_command = get_command_from_arguments(command_start, command_length, command_definitions[command_def_index]);
        if (!allocated_command.command_category) {
            free(allocated_command.name);
            return commands_array_size;
        }

        (*commands)[commands_array_size - 1] = allocated_command;  // -1 because size is indexed from 1

        first_command = false;
        command_start = command_end + 1;
        command_length = 0;
    }
    return commands_array_size;
}

Command_t *load_commands(int arg_count, char *arguments[], unsigned short *count)
{
    Command_t command_definitions[SUPPORTED_COMMANDS_COUNT];
    copy_command_definitions(command_definitions);
    Command_t *commands = allocate_commands_array();
    if (!commands)
        return NULL;

    char *commands_source = arguments[arg_count - 2];
    bool free_commands_source = false;

    if (strstr(commands_source, "-c") == commands_source)
    {
        commands_source = get_commands_from_file((commands_source + 2));
        if (!commands_source)
            return commands;
        free_commands_source = true;
    }

    short commands_count = save_commands(commands_source, command_definitions, &commands);
    if (commands_count == -1)
        return commands;
    *count = commands_count;

    if (free_commands_source)
        free(commands_source);

    return commands;
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

    unsigned short commands_count = 0;
    Command_t *commands = load_commands(arg_count, arguments, &commands_count);

    FILE *table_file = file_loader(arguments[arg_count - 1], "r");
    if (!table_file)
        return EXIT_FAILURE;

    TableSize size;
    get_table_size(table_file, delimiter, &size);

    int successful_initialization = EXIT_SUCCESS;
    Table *table = initialize_table(size, &successful_initialization);

    char **user_variables = initialize_user_variables();

    if (!successful_initialization || !user_variables)
    {
        if (!load_table(table_file, table, delimiter, size))
        {
            if (!parse_commands(table, &size, commands, commands_count, user_variables))
            {
                print_variables(user_variables);
                print_table(table, size);
                save_table(table, table_file, size, delimiter);
            }
        }
    }

    destruct_table(table, size);
    destruct_commands(commands, commands_count);
    destruct_user_variables(user_variables);
    fclose(table_file);
    return EXIT_SUCCESS;
}
