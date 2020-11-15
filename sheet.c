//
// Created by Samuel Dobroň on 10. 10. 2020.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

#define CELL_SIZE (100 + 1)  // + 1 because we need to set \0 to the end
#define ROW_BUFFER_SIZE (10240 + 2)    // +2 for \n and \0
#define COMMANDS_COUNT 26

/* COMMAND CATEGORIES */
#define TABLE_EDIT_COMMAND 1
#define DATA_PROCESSING_COMMAND 2
#define SELECTION_COMMAND 3

#define print_error(...) fprintf(stderr, __VA_ARGS__)

typedef void (*function_ptr)(); // pointer to function with no strict arguments

typedef struct{
    long start;
    long end;
    float value;
    char *text_value;
}CommandData;

typedef struct{
    char *name;
    CommandData data;
    function_ptr processing_function;   // saving pointer to function
}Command;

typedef struct {
    char *name;
    int arguments;
    int command_category;   // type of command (table edit/row selection/data processing)
    function_ptr processing_function;
}CommandDefinition;


bool compare_strings(char *first, char *second)
{
    return strcmp(first, second) == 0 ? true : false;
}

int count_delimiters(char *row, char delimiter)
{
    int delims = 0;
    int row_length = (int)strlen(row);
    for (int position = 0; position < row_length; position++)
        if (row[position] == delimiter)
            delims++;

    return delims;
}

bool last_row()
{
    int next_character = getc(stdin);
    if (next_character == EOF)
        return true;
    else
        ungetc(next_character, stdin);  // returns loaded character back to stdin
    return false;
}

void copy_to_array(char *dest, char *source, int how_many_characters)
{
    strncpy(dest, source, how_many_characters);
    dest[how_many_characters] = 0;
}

// Function caller will call function in the same way as commands from user are called
void function_caller(char *row, long start, char *text, char delimiter, function_ptr function)
{
    CommandData data = {0};
    data.start = start;
    data.end = -1;
    data.text_value = text;
    function(row, &data, &delimiter);
}

char get_cells_delimiter(char *row, char *delimiters, int remaining_lenght)  // using delimiter_argument to check if not contains -d, in this case, delimiter is " "
{
    char cell_delimiter = 0;

    int position = 0;
    while (cell_delimiter == 0 && position < remaining_lenght && remaining_lenght > 0)
    {
        if (strchr(delimiters, row[position]) != NULL)
            cell_delimiter = row[position];
        position++;
    }

    return cell_delimiter;
}

/* function will set pointers cell_start and cell_end at N cell start and cell end in row array */
int get_cell_borders(char *row, char **start, char delimiter, int wanted_column)
{
    int actual_column = 0;
    int cell_length = -1;
    int row_length = (int)strlen(row);
    int cell_start_index = -1;

    for (int position = 0; position < row_length; position++)
    {
        if ((!wanted_column || (wanted_column == (actual_column + 1) && row[position] == delimiter)) && cell_start_index == -1)
            cell_start_index = position + (!wanted_column ? 0 : 1);     // that condition won't move cell_start_index at first (0.) column behind first character

        if ((wanted_column == actual_column && row[position] == delimiter && cell_start_index > -1) || (row[position] == '\n')) {
            cell_length = position - cell_start_index;
            break;
        }
        if (row[position] == delimiter)
            actual_column++;
    }

    *start = &row[cell_start_index];
    return cell_length;
}

// return 1 when delimiter is valid; 2 when delimiter is defined but not valid nad 0 when no delimiter is defined
int valid_delimiter(int args_count, char *arguments[])
{
    if (args_count >= 2 && (compare_strings(arguments[1], "-d") && (args_count == 2 || (args_count > 2 && strlen(arguments[2]) == 0)))) {
        print_error("Invalid delimiter argument!\n");
        return 2;
    }

    if (args_count >=2 && compare_strings(arguments[1], "-d") && strlen(arguments[2]) > 0)
        return 1;
    return 0;
}

bool line_is_too_long(char *row)
{
    char *found_new_line = strchr(row, '\n');
    if (found_new_line == NULL)
    {
        print_error("Line is too long! Maximum size: %d\n", (ROW_BUFFER_SIZE - 2));   // -2 because of \n\0
        return true;
    }
    return false;
}

/* SELECTION COMMANDS - returns true in case, row could be processed */
void rows(long row_index, CommandData command, bool *can_process)
{
    if ((command.start > -1 && row_index >= command.start &&        // starting row should be defined everywhere except "- -"
         ((command.end > -1 && row_index <= command.end) ||      // end is defined
          (command.text_value && compare_strings(command.text_value, "-")))) ||   // end is defined as end of table
        // just last row to process:
        (command.start == -2 && command.end == -2 &&
         command.text_value && compare_strings(command.text_value, "-") &&
         last_row()))
        *can_process = true;
}

bool string_selection_commands(char *row, CommandData *command, char delimiter, bool begins_with_function)
{
    char *cell_start;
    int cell_size = get_cell_borders(row, &cell_start, delimiter, (int)(command->start));
    char *cell_end = cell_start + cell_size;

    char *text_beginning = strstr(cell_start, command->text_value);
    if (text_beginning != NULL && cell_end >= (text_beginning + strlen(command->text_value)) &&  // found substr in range from cell begin to cell end
        ((cell_start == text_beginning && begins_with_function) ||                 // "beginswith" command
         (cell_start < text_beginning && !begins_with_function)))                   // "contains" command
        return true;
    return false;
}

void beginswith(char *row, CommandData *command, const char *delimiter, bool *can_start)
{
    *can_start = string_selection_commands(row, command, *delimiter, true);
}

void contains(char *row, CommandData *command, const char *delimiter, bool *can_start)
{
    *can_start = string_selection_commands(row, command, *delimiter, false);
}

bool process_selection_commands(char *row, Command *commands, int commands_count, char delimiter, long row_index)
{
    bool can_process = false;
    for (int command_index = 0; command_index < commands_count; command_index++)
    {
        function_ptr command_processing_function = commands[command_index].processing_function;
        if (command_processing_function == rows)
            command_processing_function(row_index, commands[command_index].data, &can_process);
        else
            command_processing_function(row, &commands[command_index].data, &delimiter, &can_process);    // beginswith and contains commands
        if (can_process)
            break;
    }
    return can_process;
}

int check_column_requirements(size_t column_size, int column_index, int column_count, long row_index, bool last_column)
{
    if (column_size >= CELL_SIZE) {
        print_error("Column is bigger than allowed!\n");
        return EXIT_FAILURE;
    }else if (last_column && row_index && column_index + 1 != column_count){  // +1 because column_index is increased after this check
        print_error("You have inconsistent column count!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool last_cell(char *cell_start, int cell_size)
{
    char *cell_end = cell_start + cell_size;
    char *n_position = strchr(cell_start, '\n');
    if (n_position != NULL && n_position <= cell_end)
        return true;
    return false;
}

int parse_line(char *raw_line, char *delimiter, long row_index, int *columns_count)
{
    char *cell_end = raw_line;
    int column_index = 0;
    int remaining_row_length;

    while ((remaining_row_length = (int)strlen(cell_end)) > 0)
    {
        char *cell_start = cell_end;      // set end of cell before as start of actual cell
        char actual_delimiter = get_cells_delimiter(cell_end, delimiter, remaining_row_length);

        cell_end = strchr(cell_end, actual_delimiter);
        int cell_length = (int)(cell_end - cell_start);

        bool last_column = last_cell(cell_start, cell_length);

        int column_requirements = check_column_requirements(cell_length, column_index, *columns_count, row_index, last_column);
        if (column_requirements)
            return column_requirements;

        column_index++;
        if (!actual_delimiter || cell_end == NULL || !strlen(cell_end))
            break;

        cell_end[0] = delimiter[0];     // replace delimiter with correct one
        cell_end++;                     // move pointer behind delimiter
    }

    if (!row_index)  // set column count from first row
        *columns_count = column_index;

    return EXIT_SUCCESS;
}


long get_valid_row_number(char *number, int allow_dash)
{
    if (allow_dash && number != NULL && compare_strings(number, "-"))
        return 0;

    errno = 0;
    char *remaining_start;
    long converted = strtol(number, &remaining_start, 10);

    if ((number != remaining_start && converted > 0 && *remaining_start == '\0') &&
        (converted != LONG_MIN && converted != LONG_MAX) && errno != ERANGE)
    {
        return converted;
    }else
        return -1;
}

int get_command_definition(char *command_name, CommandDefinition *command_definitions)
{
    for (int command_def_index = 0; command_def_index < COMMANDS_COUNT; command_def_index++) {
        if (compare_strings(command_name, command_definitions[command_def_index].name))
            return command_def_index;
    }
    return -1;
}

int get_commands_count(char *arguments[], int args_count, CommandDefinition *definitions, int command_category)
{
    int count = 0;
    for (int arg_index = 0; arg_index < args_count; arg_index++)
    {
        int command_def_index = get_command_definition(arguments[arg_index], definitions);
        if (command_def_index == -1)
            continue;
        int category = definitions[command_def_index].command_category;
        if (category == command_category)
            count++;
    }
    return count;
}

void acol(char *row, CommandData *command_data, const char *delimiter)
{
    (void)command_data; // command data is not used here, but it has to be here
    int row_ending = (int)strlen(row) - 1;  // -1 because of \n at the end
    row[row_ending] = *delimiter;
    strcpy(&row[row_ending + 1], "\n"); // strcpy will add \0
}

void dcol(char *row, CommandData *command_data, const char *delimiter)
{
    char *cell_start;
    int cell_length = get_cell_borders(row, &cell_start, *delimiter, (int)command_data->start);
    char *continue_at = cell_start + cell_length;
    if (continue_at[0] != '\n') // will move pointer behind delimiter but not in last column - there is no next delimiter
        continue_at++;
    else
        cell_start--;   // remove delimiter in case, the last cell is being deleted
    memmove(cell_start,  continue_at, strlen(cell_start));
}

void dcols(char *row, CommandData *command_data, const char *delimiter)
{
    char *start;
    char *end;
    get_cell_borders(row, &start, *delimiter, (int)command_data->start);
    int end_cell_length = get_cell_borders(row, &end, *delimiter, (int)command_data->end);

    end += end_cell_length; // move pointer to end of cell
    if (end[0] != '\n')     // in case, it is not the last column, move end of column behind delimiter
        end++;
    else
        start--;

    memmove(start, end, strlen(end) + 1);
}

void icol(char *row, CommandData *command_data, const char *delimiter)
{
    char *cell_start;
    get_cell_borders(row, &cell_start, *delimiter, (int)command_data->start);
    memmove(cell_start + 1, cell_start, strlen(cell_start) + 1);
    *cell_start = *delimiter;
}

void cset(char *row, CommandData *command, const char *delimiter)
{
    char *actual_column;
    int actual_column_length = get_cell_borders(row, &actual_column, *delimiter, (int)command->start);

    int new_value_length = (int)strlen(command->text_value);
    int offset = new_value_length - actual_column_length;   // calculate direction and offset of move
    char *actual_column_end = actual_column + actual_column_length;

    memmove(actual_column_end + offset, actual_column_end, strlen(actual_column) + 1);  // +1 for copy ending 0
    strncpy(actual_column, command->text_value, new_value_length);   // command->text value contains \0, so its necessary to copy characters until \0
}

void irow(char *row, CommandData *command_data, long row_index, const char *delimiter)
{
    if (command_data->start != row_index)
        return;

    int columns_count = count_delimiters(row, *delimiter);
    for (int delimiters = 0; delimiters < columns_count; delimiters++)
        printf("%c", *delimiter);

    printf("\n");
}

void arow(char *row, CommandData *command_data, long row_index, const char *delimiter)
{
    if (!last_row())
        return;
    (void)command_data;
    CommandData data = {0};
    data.start = row_index;
    irow(row, &data, row_index, delimiter);
}

void arow_caller(int edit_commands_count, Command *edit_commands, char *row_buffer, char delimitier)
{
    for (int command = 0; command < edit_commands_count; command++)
        if (edit_commands[command].processing_function == arow)
            arow(row_buffer, &edit_commands[command].data, LONG_MAX, &delimitier);
}

/* implementation od drow and drows commands */
void drow_s(char *row, CommandData *command_data, long row_index)
{
    if ((row_index == command_data->start && command_data->end == -1) ||        // drow command
        (row_index >= command_data->start && row_index <= command_data->end))   // drows command
        row[0] = '\0';
}

void process_commands(char *row, Command *edit_commands, int edit_commands_count, char delimiter, long row_index)
{
    for (int command_index = 0; command_index < edit_commands_count; command_index++)
    {
        function_ptr edit_function = edit_commands[command_index].processing_function;
        if (edit_function == arow)  // arow has custom calling
            continue;
        if (edit_function == drow_s)
            edit_function(row, &edit_commands[command_index].data, row_index);
        else if (edit_function == irow)
            edit_function(row, &edit_commands[command_index].data, row_index, &delimiter);
        else
            edit_function(row, &edit_commands[command_index].data, &delimiter);
        if (row[0] == '\0') // row has been deleted, dont need to process another commands
            break;
    }
}

long get_valid_column_number(char *text_form)    // will return -1 for invalid col num
{
    //int number = -1;
    long column_number = get_valid_row_number(text_form, false);
    return column_number -1;
}

int set_command_data(char **arguments, int command_index, CommandData *command_data, CommandDefinition *command_definition)
{
    int arg_count = command_definition->arguments;
    long start = -1;
    long end = -1;
    float value = -1;
    char *text_value = NULL;

    function_ptr function = command_definition->processing_function;
    bool string_value_function = (function == contains || function == beginswith || function == cset);

    if (arg_count >= 1)
        start = get_valid_column_number(arguments[command_index + 1]);
    if (arg_count >= 2)
        end = get_valid_column_number(arguments[command_index + 2]);
    if ((string_value_function || function == rows) && strlen(arguments[command_index + 2]) < CELL_SIZE)
        text_value = arguments[command_index + 2];  // at this place, will be saved text value
    if (arg_count >= 3)
        value = (float)get_valid_column_number(arguments[command_index + 3]);

    if ((((start == -2 || end == -2) && (text_value == NULL ||                          // totally invalid input data
            (text_value != NULL && start == -2 && end == -2))) && function != rows) ||  // invalid input data in string commands, rows has specific arguments
        (start == -1 && end == -1 && value == -1 && text_value == NULL && function != arow) ||  // empty input data, arow is processed in another way - it's excluded
        (text_value == NULL && (string_value_function)))  // functions that require text value
    {
        print_error("Invalid arguments!\n");
        return EXIT_FAILURE;
    }

    command_data->start = start;
    command_data->end = end;
    command_data->value = value;
    command_data->text_value = text_value;
    return EXIT_SUCCESS;
}

int missing_command_arguments(int command_arguments, int args_count, int command_index)
{
    if (command_arguments > (args_count - command_index - 1)) {   // -1 because of actual processing command
        print_error("Invalid arguments count!\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int get_commands(int args_count, char *arguments[], CommandDefinition *command_definitions, Command *commands, int processing_commands_category)
{
    int edit_command_index = 0;
    for (int command_index = 1; command_index < args_count;)    // starting at 1, 0. arg is program name
    {
        int command_def_index = get_command_definition(arguments[command_index], command_definitions);
        if (command_def_index == -1 ||  // check command validity
            (command_def_index > -1 && processing_commands_category != command_definitions[command_def_index].command_category)) {    // check command category
            command_index++;
            continue;
        }

        CommandDefinition command_definition = command_definitions[command_def_index];
        if (missing_command_arguments(command_definition.arguments, args_count, command_index))
            return EXIT_FAILURE;

        CommandData data = {0};
        int setting_data_result = set_command_data(arguments, command_index, &data, &command_definition);
        if (setting_data_result)
            return EXIT_FAILURE;
        Command command = {arguments[command_index], data, command_definition.processing_function};

        commands[edit_command_index] = command;
        command_index += command_definition.arguments + 1;
        edit_command_index++;
    }
    return EXIT_SUCCESS;
}

/* common function for commands tolower and toupper, they are almost same */
void column_case(char *row, CommandData *commandData, const char *delimiter, bool lower)
{
    char *start;
    int column_length = get_cell_borders(row, &start, *delimiter, (int)commandData->start);
    for (int position = 0; position < column_length; position++)
        start[position] = (char)(lower ? tolower(start[position]) : toupper(start[position]));
}

void column_tolower(char *row, CommandData *command, char *delimiter)
{
    column_case(row, command, delimiter, true);
}

void column_toupper(char *row, CommandData *command, char *delimiter)
{
    column_case(row, command, delimiter, false);
}

void copy(char *row, CommandData *command, const char *delimiter)
{
    char *copy_from;
    int cell_length = get_cell_borders(row, &copy_from, *delimiter, (int)(command->start));
    char to_copy[cell_length + 1];
    copy_to_array(to_copy, copy_from, cell_length);

    function_caller(row, command->end, to_copy, *delimiter, cset);
}

void swap(char *row, CommandData *command, const char *delimiter)
{
    char *what;
    char *with;
    int what_size = get_cell_borders(row, &what, *delimiter, (int)(command->start));
    int with_size = get_cell_borders(row, &with, *delimiter, (int)(command->end));

    char what_temp[what_size + 1];
    copy_to_array(what_temp, what, what_size);

    char with_temp[with_size + 1];
    copy_to_array(with_temp, with, with_size);

    if (what_temp[what_size - 1] == '\n')
        what_temp[what_size - 1] = '\0';
    if (with_temp[with_size - 1] == '\n')
        with_temp[with_size - 1] = '\0';

    function_caller(row, command->start, with_temp, *delimiter, cset);
    function_caller(row, command->end, what_temp, *delimiter, cset);
}

void move(char *row, CommandData *command, const char *delimiter)
{
    char *cell_source;
    int source_cell_length = get_cell_borders(row, &cell_source, *delimiter, (int)(command->start));
    long move_to = command->end;
    long move_from = command->start;

    char temp_cell[source_cell_length + 1]; // using temp cell because of moving columns when whole row is used
    copy_to_array(temp_cell, cell_source, source_cell_length);

    function_caller(row, move_to, NULL, *delimiter, icol);

    if (move_from > move_to)
        move_from++;
    else if (move_from < move_to)
        move_to--;  // move column to set temp value before destination cell

    function_caller(row, move_from, NULL, *delimiter, dcol);
    function_caller(row, move_to, temp_cell, *delimiter, cset);
}

bool get_numeric_cell_value(char *column, float *value)
{
    errno = 0;
    char *result;
    *value = strtof(column, &result);

    if (errno != 0 || (result != NULL && result[0] != '\0'))
        return false;
    return true;
}

void set_numeric_value_to_cell(float value, long cell_index, const char *delimiter, char *row)
{
    char *cell_to_set_start;
    get_cell_borders(row, &cell_to_set_start, *delimiter, (int)cell_index);

    int no_of_digits = snprintf(NULL, 0, "%f", value);
    char result[no_of_digits];
    result[no_of_digits + 1] = '\0';
    sprintf(result,"%g", value);

    function_caller(row, cell_index, result, *delimiter, cset);
}

/* Common function for csum, cavg, cmin, cmax functions. Function has parameter "what_to_do" which defines what to do with numbers */
void cx_commands(char *row, CommandData *command, const char *delimiter, int what_to_do)
{
    float result = 0;
    float number_to_add = 0;
    float valid_columns = 0;
    int end_column_index = (int)command->value;

    for (int column = (int)command->end; column <= end_column_index; column++)
    {
        char *column_start;
        int column_length = get_cell_borders(row, &column_start, *delimiter, column);
        char cell[column_length + 1];
        copy_to_array(cell, column_start, column_length);

        bool valid_number = get_numeric_cell_value(cell, &number_to_add);
        if (!valid_number)
            continue;

        if (what_to_do <= 2)
            result += number_to_add;
        else if ((what_to_do == 3 && number_to_add < result) || (what_to_do == 4 && number_to_add > result))
            result = number_to_add;

        valid_columns++;
    }
    if (what_to_do == 2)
        result /= valid_columns ? valid_columns : 1;    // prevent / by 0
    else if (what_to_do == 5)
        result = valid_columns;

    set_numeric_value_to_cell(result, command->start, delimiter, row);
}

void csum(char *row, CommandData *command, const char *delimiter)
{
    cx_commands(row, command, delimiter, 1);
}

void cavg(char *row, CommandData *command, const char *delimiter)
{
    cx_commands(row, command, delimiter, 2);
}

void cmin(char *row, CommandData *command, const char *delimiter)
{
    cx_commands(row, command, delimiter, 3);
}
void cmax(char *row, CommandData *command, const char *delimiter)
{
    cx_commands(row, command, delimiter, 4);
}

void ccount(char *row, CommandData *command, const char *delimiter)
{
    cx_commands(row, command, delimiter, 5);
}

void cseq(char *row, CommandData *command, const char *delimiter)
{
    int seq_start = (int)command->value + 1; // +1 because parsing decreased it by 1
    int starting_column = (int)command->start;
    for (int column = starting_column; column <= command->end; column++)
        set_numeric_value_to_cell((float)(seq_start+(column-starting_column)), column, delimiter, row); // add # of iteration to seq_start, to increase it
}

void column_round(char *row, CommandData *command, const char *delimiter)
{
    char *column;
    int cell_length = get_cell_borders(row, &column, *delimiter, (int)(command->start));
    char cell[cell_length + 1];
    copy_to_array(cell, column, cell_length);

    double to_round;
    char *remaining = NULL;
    errno = 0;

    to_round = strtod(cell, &remaining);
    if (errno != 0 || (remaining != NULL && *remaining != '\0'))
        return;

    int rounded = (int)(to_round + (to_round > 0.0 ? 0.5 : -0.5));
    set_numeric_value_to_cell((float)rounded, (int)(command->start), delimiter, row);
}

void column_int(char *row, CommandData *command, const char *delimiter)
{
    char *column_start;
    int cell_length = get_cell_borders(row, &column_start, *delimiter, (int)(command->start));
    char *cell_end = column_start + cell_length;

    char *decimal_dot = strchr(column_start, '.');
    if (decimal_dot > cell_end || !decimal_dot)   // dot found behind actual cell
        return;

    memmove(decimal_dot, cell_end, strlen(cell_end) + 1);
}

void add_missing_columns(char *row, char delimiter, CommandData *command)
{
    size_t row_len = strlen(row) - 1;
    size_t position = row_len;

    long to_add;    // calculate how many delims needs to add for match delims count in first row
    if (command->end != -1)
        to_add = command->end - count_delimiters(row, delimiter);
    else
        to_add = ROW_BUFFER_SIZE - (CELL_SIZE + 1) - row_len;

    for (; position < to_add + row_len; position++)
        row[position] = delimiter;

    row[position] = '\n';
    row[position+1] = '\0';
}

void split(char *row, CommandData *command, const char *delimiter)
{
    /* In valid row could be in cell maximum of 100 (CELL_SIZE) new delimiters, so 100 last characters are reserved to keep column count in whole table same.
     * Because in table could be added up to 100 new delimiters (columns splitted by splitting character). */
    if (strlen(row) > (ROW_BUFFER_SIZE - CELL_SIZE + 1)) {  // CELL_SIZE have -1, so we need to add it
        print_error("Split is supported up to %d characters per line only.\n", (ROW_BUFFER_SIZE - CELL_SIZE + 1));
        return;
    }
    char *cell_to_split;
    int cell_length = get_cell_borders(row, &cell_to_split, *delimiter, (int)(command->start));
    char splitter = command->text_value[0];

    for (int position = 0; position < cell_length; position++)
        if (cell_to_split[position] == splitter)
            cell_to_split[position] = *delimiter;

    add_missing_columns(row, *delimiter, command);  // add delimiters to set same column count for every line
    if (command->end == -1)
    {
        int new_table_columns_count = count_delimiters(row, *delimiter);
        command->end = new_table_columns_count;
    }
}

void get_all_command_definitions(CommandDefinition *commands)
{
    CommandDefinition base_commands[COMMANDS_COUNT] = {
            /* TABLE EDIT COMMANDS */
            {"irow",    1, TABLE_EDIT_COMMAND, irow},
            {"arow",    0, TABLE_EDIT_COMMAND, arow},
            {"drow",    1, TABLE_EDIT_COMMAND, drow_s}, // osetrit pripad, ked nastavim csetom
            {"drows",   2, TABLE_EDIT_COMMAND, drow_s},
            {"icol",    1, TABLE_EDIT_COMMAND, icol},
            {"acol",    0, TABLE_EDIT_COMMAND, acol},
            {"dcol",    1, TABLE_EDIT_COMMAND, dcol},
            {"dcols",   2, TABLE_EDIT_COMMAND, dcols},
            /* DATA PROCESSING COMMANDS */
            {"cset",    2, DATA_PROCESSING_COMMAND, cset},
            {"tolower", 1, DATA_PROCESSING_COMMAND, column_tolower},
            {"toupper", 1, DATA_PROCESSING_COMMAND, column_toupper},
            {"round",   1, DATA_PROCESSING_COMMAND, column_round},
            {"int",     1, DATA_PROCESSING_COMMAND, column_int},
            {"copy",    2, DATA_PROCESSING_COMMAND, copy},
            {"swap",    2, DATA_PROCESSING_COMMAND, swap},
            {"move",    2, DATA_PROCESSING_COMMAND, move},
            {"csum",    3, DATA_PROCESSING_COMMAND, csum},
            {"cavg",    3, DATA_PROCESSING_COMMAND, cavg},
            {"cmin",    3, DATA_PROCESSING_COMMAND, cmin},
            {"cmax",    3, DATA_PROCESSING_COMMAND, cmax},
            {"ccount",  3, DATA_PROCESSING_COMMAND, ccount},
            {"cseq",    3, DATA_PROCESSING_COMMAND, cseq},
            /* ROW SELECTION COMMANDS */
            {"rows",        2, SELECTION_COMMAND, rows},
            {"beginswith",  2, SELECTION_COMMAND, beginswith},
            {"contains",    2, SELECTION_COMMAND, contains},
            /* PREMIUM COMMANDS */
            {"split", 2, DATA_PROCESSING_COMMAND, split}
    };
    for (int command_index = 0; command_index < COMMANDS_COUNT; command_index++)
        commands[command_index] = base_commands[command_index];
}

void print_row(char *row)
{
    size_t row_length = strlen(row);
    if (row_length == 1 && row[0] == '\n')
        return;
    printf("%s", row);
}

int main(int args_count, char *arguments[])
{
    /* GET DELIMITER */
    int defined_valid_delimiter = valid_delimiter(args_count, arguments);
    char default_delimiter[] = {" "};
    char *cells_delimiter = default_delimiter;
    if (defined_valid_delimiter == 2)
        return EXIT_FAILURE;
    else if (defined_valid_delimiter)
        cells_delimiter = arguments[2];

    CommandDefinition command_definitions[COMMANDS_COUNT];
    get_all_command_definitions(command_definitions);

    /* COMMANDS COUNT */
    int edit_commands_count         =   get_commands_count(arguments, args_count, command_definitions, TABLE_EDIT_COMMAND);
    int processing_commands_count   =   get_commands_count(arguments, args_count, command_definitions, DATA_PROCESSING_COMMAND);
    int selection_commands_count    =   get_commands_count(arguments, args_count, command_definitions, SELECTION_COMMAND);

    /* PREPARE SELECTION COMMANDS */
    Command selection_commands[selection_commands_count];
    int selection_commands_parsing_result = get_commands(args_count, arguments, command_definitions, selection_commands, SELECTION_COMMAND);
    if (selection_commands_parsing_result)
        return selection_commands_parsing_result;

    /*  PREPARE TABLE EDITING COMMANDS  */
    Command edit_commands[edit_commands_count];
    int edit_commands_parsing_result = get_commands(args_count, arguments, command_definitions, edit_commands, TABLE_EDIT_COMMAND);
    if (edit_commands_parsing_result)
        return selection_commands_parsing_result;

    /* PREPARE DATA PROCESSING COMMANDS */
    Command processing_commands[processing_commands_count];
    int processing_commands_parsing_result = get_commands(args_count, arguments, command_definitions, processing_commands, DATA_PROCESSING_COMMAND);
    if (processing_commands_parsing_result)
        return processing_commands_parsing_result;

    long row_index = -1;  // using long because max number of rows is not defined
    int column_count = 0;
    char row_buffer[ROW_BUFFER_SIZE];

    while (fgets(row_buffer, ROW_BUFFER_SIZE, stdin) && !line_is_too_long(row_buffer))
    {
        row_index++;

        int parsing_result = parse_line(row_buffer, cells_delimiter, row_index, &column_count);
        if (parsing_result)
            return parsing_result;

        if (!selection_commands_count || process_selection_commands(row_buffer, selection_commands, selection_commands_count, *cells_delimiter, row_index))
        {
            process_commands(row_buffer, edit_commands, edit_commands_count, cells_delimiter[0], row_index);
            process_commands(row_buffer, processing_commands, processing_commands_count, cells_delimiter[0], row_index);
        }
        print_row(row_buffer);
    }
    arow_caller(edit_commands_count, edit_commands, row_buffer, cells_delimiter[0]);

    return EXIT_SUCCESS;
}
