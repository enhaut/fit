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

// error codes
#define ERROR_BIGGER_COLUMN_THAN_ALLOWED 1
#define ERROR_MAXIMUM_COLUMN_LIMIT_REACHED 2
#define ERROR_INCONSISTENT_COLUMNS 3
#define ERROR_INVALID_COMMAND_USAGE 4
#define ERROR_MAXIMUM_ROW_SIZE_REACHED 5


typedef struct{
    char command[10 + 1];       // 10 is length of longest command, + 1 for \0
    long starting_row;          // -1 is unused command or invalid input, 0 is reserved for "rows" and means "-" in input
    long ending_row;            // used in "rows" command only, can contains value, -1 for invalid or unused, 0 means "-" in input
    char row_match[CELL_SIZE];  // used in "beginswith" and "contains" commands only
}SelectionRowCommand;

typedef void (*function_ptr)(); // pointer to function with no strict arguments

typedef struct{
    long start;
    long end;
    int value;
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

bool is_defined_delimiter(int args_count, char **arguments)
{
    bool is_defined_delimiter = false;
    if (args_count >= 3 && compare_strings(arguments[1], "-d") && strlen(arguments[2]) > 0)
        is_defined_delimiter = true;

    return is_defined_delimiter;
}

int check_column_requirements(size_t column_size, int column_index, int column_count, long row_index, const char *remaining_row)
{
    int return_code = 0;
    if (column_size > CELL_SIZE)
    {
        fprintf(stderr, "Column is bigger than allowed!\n");
        return_code = ERROR_BIGGER_COLUMN_THAN_ALLOWED;
    }else if (column_index + 1 != column_count && row_index > 0 && remaining_row == NULL){      // TODO: fix!!!
        // +1 because column index is indexed from 0 and column_count from 1, checking of remaining_row to make sure that actual column is the last one
        fprintf(stderr, "You have inconsistent column count!\n");
        return_code = ERROR_INCONSISTENT_COLUMNS;
    }
    return return_code;
}


bool can_process_row(SelectionRowCommand *selection_commands, long row_index, char parsed_row[][CELL_SIZE], bool last_row)
{
    bool can_process = true;

    for (int command_index = 0; command_index < 3; command_index++)
    {
        if (selection_commands[command_index].starting_row < 0)  // valid starting row is required in every command
            continue;
        
        long starting_row = selection_commands[command_index].starting_row;
        long ending_row = selection_commands[command_index].ending_row;
        char *command_name = selection_commands[command_index].command;

        if (compare_strings(selection_commands[command_index].command, "rows"))
        {
            if ((!starting_row && !ending_row && last_row) ||   // row indexes are "-"
                (starting_row && starting_row <= row_index &&   // checking starting row
                (!ending_row || ending_row >= row_index)))      // checking ending row
                can_process = true;
            else
                can_process = false;
        }else{
            char *remaining;
            remaining = strstr(parsed_row[starting_row - 1], selection_commands[command_index].row_match);  // -1 because parsed_row is indexing from 0

            if (remaining != NULL)
            {
                if (compare_strings(command_name, "contains") ||
                    (compare_strings(command_name, "beginswith") &&
                     strlen(remaining) == strlen(parsed_row[starting_row -1])))
                    can_process = can_process == true ? true : false;
                else
                    can_process = false;
            }else
                can_process = false;
        }
    }
    return can_process;
}

int parse_line(char *raw_line, char *delimiter, long row_index, int *columns_count)
{
    char *cell_end = raw_line;
    int column_index = 0;
    int remaining_row_length;

    while ((remaining_row_length = (int)strlen(cell_end)) > 1)
    {
        char *cell_start = cell_end;      // set end of cell before as start of actual cell
        char actual_delimiter = get_cells_delimiter(cell_end, delimiter, remaining_row_length);
        if (actual_delimiter == 0)
            break;

        cell_end = strchr(cell_end, actual_delimiter);
        int cell_length = (int)(cell_end - cell_start);

        if (cell_end == NULL || !strlen(cell_end))
            break;

        int column_requirements = check_column_requirements(cell_length, column_index, *columns_count, row_index, cell_end);
        if (column_requirements > 0)
            return column_requirements;

        cell_end[0] = delimiter[0];     // replace delimiter with correct one
        cell_end++;                     // move pointer behind delimiter
        column_index++;
    }

    if (row_index == 0)  // set column count from first row
        *columns_count = column_index;

    return 0;
}


long get_valid_row_number(char *number, int allow_dash)
{
    if (compare_strings(number, "-") && allow_dash)
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

// function for parsing selection commands using strings (contains, beginswith)
int process_string_selection_commands(SelectionRowCommand *command, char *row_match)
{
    size_t should_contain_text_length = strlen(row_match);

    if (0 < should_contain_text_length && should_contain_text_length < CELL_SIZE)
        strcpy(command->row_match, row_match);
    else{
        fprintf(stderr, "Invalid syntax of command. Usage: beginswith/contains [from row] [should contains]\n");
        return ERROR_INVALID_COMMAND_USAGE;
    }
    return 0;
}

int get_selection_commands(int args_count, char *arguments[], SelectionRowCommand *commands)
{
    bool started_with_selection_commands = false;
    for (int command_index = 0; command_index < args_count;)  // using custom incrementing to skip command values
    {
        int saving_index;
        if (compare_strings(arguments[command_index], "rows"))
            saving_index = 0;
        else if (compare_strings(arguments[command_index], "beginswith"))
            saving_index = 1;
        else if (compare_strings(arguments[command_index], "contains"))
            saving_index = 2;
        else{
            if (started_with_selection_commands)
                break;  // prevent to parse not row selection commands, it is useless here
            command_index++;
            continue;
        }

        if (!started_with_selection_commands)  // mark that, program is parsing selection commands at this moment
            started_with_selection_commands = true;

        long starting_row = get_valid_row_number(arguments[command_index + 1], !saving_index);  // "rows" has saving_index 0 so !0 is 1
        commands[saving_index].starting_row = starting_row;

        if (starting_row == -1)
        {
            fprintf(stderr, "Invalid syntax of command!\n");
            return ERROR_INVALID_COMMAND_USAGE;
        }

        if (saving_index == 0)
        {
            long ending_row = get_valid_row_number(arguments[command_index + 2], 1);
            commands[saving_index].ending_row = ending_row;

            if ((starting_row > ending_row && ending_row != 0) ||   // check it only if ending row is not "-"
                ending_row == -1 ||                                 // invalid ending_row index
                (starting_row == 0 && ending_row > 0))              // "-" in starting row input but valid ending row index is set
            {
                fprintf(stderr, "Invalid ending row index!\n");
                return ERROR_INVALID_COMMAND_USAGE;
            }
        }else{
            int result = process_string_selection_commands(&commands[saving_index], arguments[command_index + 2]);
            if (result > 0)
                return result;
        }
        command_index += 3;  // +3 to move to next selection commands, every selection command has 1 command and 2 values
    }
    return 0;
}

int get_command_definition(char *command_name, CommandDefinition *command_definitions)
{
    for (int command_def_index = 0; command_def_index < COMMANDS_COUNT; command_def_index++) {
        if (compare_strings(command_name, command_definitions[command_def_index].name))
            return command_def_index;
    }
    return -1;
}

void get_command_counts(int args_count, char **arguments, int *table_edit_cmds, int *data_processing_cmds, CommandDefinition *commands)   // TODO: add selection commands
{
    for (int arg_index = 0; arg_index < args_count; arg_index++)
    {
        int command_def_index = get_command_definition(arguments[arg_index], commands);
        if (command_def_index == -1)
            continue;
        int category = commands[command_def_index].command_category;
        int *commands_counter = category == 1 ? table_edit_cmds : data_processing_cmds;
        (*commands_counter)++;
    }
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

void empty_function(const char *row, CommandData *command_data)   // function used to set some function to command so mark command as used and should be performed
{
    (void)row;
    (void)command_data;
}

int get_valid_column_number(char *text_form)    // will return -1 for invalid col num
{
    int number = -1;
    return (int)get_valid_row_number(text_form, false) + number;
}

void set_command_data(char **arguments, int command_index, CommandData *command_data, CommandDefinition *command_definition)
{
    int arg_count = command_definition->arguments;
    long start = -1;
    long end = -1;
    int value = -1;
    char *text_value = NULL;

    if (arg_count >= 1)
        start = get_valid_column_number(arguments[command_index + 1]);
    if (arg_count >= 2)
        end = get_valid_column_number(arguments[command_index + 2]);
    if (end < 0 && arg_count == 2)
        text_value = arguments[command_index + 2];  // at this place, will be saved text value

    if (arg_count >= 3)
        value = get_valid_column_number(arguments[command_index + 3]);

    command_data->start = start;
    command_data->end = end;
    command_data->value = value;
    command_data->text_value = text_value;
}

int get_commands(int args_count, char *arguments[], CommandDefinition *command_definitions, Command *commands, int processing_commands_category)
{
    int minimal_arguments = 0;
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
        int command_arguments = command_definition.arguments;
        if (command_index + command_arguments > args_count)
            return -ERROR_INVALID_COMMAND_USAGE;    // TODO: *-1 to error codes

        CommandData data = {0};
        set_command_data(arguments, command_index, &data, &command_definition);
        Command command = {arguments[command_index], data, command_definition.processing_function};

        commands[edit_command_index] = command;
        command_index += command_arguments + 1;
        edit_command_index++;
    }
    return minimal_arguments;
}

void cset(char *row, CommandData *command, const char *delimiter)
{
    char *actual_column;
    int actual_column_length = get_cell_borders(row, &actual_column, *delimiter, (int)command->start);

    int new_value_length = (int)strlen(command->text_value);
    int offset = new_value_length - actual_column_length;   // calculate direction and offset of move
    char *actual_column_end = actual_column + actual_column_length;

    memmove(actual_column_end + offset, actual_column_end, strlen(actual_column));
    strncpy(actual_column, command->text_value, new_value_length);   // command->text value contains \0, so its necessary to copy characters until \0
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
        if (edit_function != drow_s)
            edit_function(row, &edit_commands[command_index].data, &delimiter);
        else
            edit_function(row, &edit_commands[command_index].data, row_index);
    }
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
    to_copy[cell_length] = '\0';
    strncpy(to_copy, copy_from, cell_length);

    CommandData data = {0};
    data.text_value = to_copy;
    data.start = command->end;

    cset(row, &data, delimiter);
}

void swap(char *row, CommandData *command, char *delimtier)
{
    char *what;
    char *with;
    int what_size = get_cell_borders(row, &what, *delimtier, (int)(command->start));
    int with_size = get_cell_borders(row, &with, *delimtier, (int)(command->end));

    char what_temp[what_size + 1];
    strncpy(what_temp, what, what_size);
    what_temp[what_size] = '\0';

    char with_temp[with_size + 1];
    strncpy(with_temp, with, with_size);
    with_temp[with_size] = '\0';

    if (what_temp[what_size - 1] == '\n')
        what_temp[what_size - 1] = '\0';
    if (with_temp[with_size - 1] == '\n')
        with_temp[with_size - 1] = '\0';

    CommandData with_data = {0};
    with_data.start = command->start;
    with_data.text_value = with_temp;
    cset(row, &with_data, delimtier);

    CommandData what_data = {0};
    what_data.start = command->end;
    what_data.text_value = what_temp;
    cset(row, &what_data, delimtier);

}

void move(char *row, CommandData *command, char *delimiter)
{
    char *dest_cell;
    get_cell_borders(row, &dest_cell, *delimiter, (int)(command->end));

    char *cell_source;
    int source_cell_length = get_cell_borders(row, &cell_source, *delimiter, (int)(command->start));

    char temp_cell[source_cell_length + 1]; // using temp cell because of moving columns when whole row is used
    strncpy(temp_cell, cell_source, source_cell_length);
    temp_cell[source_cell_length] = '\0';

    CommandData to_delete_col = {0};
    to_delete_col.start = command->start;
    dcol(row, &to_delete_col, delimiter);

    CommandData to_add_col = {0};
    to_add_col.start = command->end;
    icol(row, &to_add_col, delimiter);

    memmove(dest_cell + source_cell_length, dest_cell, strlen(dest_cell));
    strncpy(dest_cell + 1, temp_cell, source_cell_length);  // +1 to move behind delimiter
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

    CommandData to_set = {0};
    to_set.start = cell_index;
    to_set.text_value = result;
    cset(row, &to_set, delimiter);
}

/* Common function for csum, cavg, cmin, cmax functions. Function has parameter "what_to_do" which defines what to do with numbers */
void cx_commands(char *row, CommandData *command, const char *delimiter, int what_to_do)
{
    float result = 0;
    float number_to_add = 0;
    float valid_columns = 0;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        char *column_start;
        int column_length = get_cell_borders(row, &column_start, *delimiter, column);
        char cell[column_length + 1];
        strncpy(cell, column_start, column_length);
        cell[column_length] = '\0';

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
    int seq_start = command->value + 1; // +1 because parsing decreased it by 1
    int starting_column = (int)command->start;
    for (int column = starting_column; column <= command->end; column++)
        set_numeric_value_to_cell((float)(seq_start+column-starting_column), column, delimiter, row); // add # of iteration to seq_start, to increase it
}

void get_all_command_definitions(CommandDefinition *commands)
{
    CommandDefinition base_commands[COMMANDS_COUNT] = {
            /* TABLE EDIT COMMANDS */
            {"irow",    1, 1, empty_function},
            {"arow",    0, 1, empty_function},
            {"drow",    1, 1, drow_s},
            {"drows",   2, 1, drow_s},
            {"icol",    1, 1, icol},
            {"acol",    0, 1, acol},
            {"dcol",    1, 1, dcol},
            {"dcols",   2, 1, dcols},
            /* DATA PROCESSING COMMANDS */
            {"cset",    2, 2, cset},
            {"tolower", 1, 2, column_tolower},
            {"toupper", 1, 2, column_toupper},
            {"copy",    2, 2, copy},
            {"swap",    2, 2, swap},
            {"move",    2, 2, move},
            {"csum",    3, 2, csum},
            {"cavg",    3, 2, cavg},
            {"cmin",    3, 2, cmin},
            {"cmax",    3, 2, cmax},
            {"ccount",  3, 2, ccount},
            {"cseq",    3, 2, cseq},
            {"rseq",    4, 2, empty_function},
            {"rsum",    3, 2, empty_function},
            {"ravg",    3, 2, empty_function},
            {"rmin",    3, 2, empty_function},
            {"rmax",    3, 2, empty_function},
            {"rcount",  3, 2, empty_function},
            /* ROW SELECTION COMMANDS */
            // TODO
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
    bool defined_custom_delimiter = is_defined_delimiter(args_count, arguments);
    char default_delimiter[] = {" "};
    char *cells_delimiter = default_delimiter;
    if (defined_custom_delimiter)
        cells_delimiter = arguments[2];

    CommandDefinition command_definitions[COMMANDS_COUNT];
    get_all_command_definitions(command_definitions);

    /* PREPARE SELECTION COMMANDS */
    SelectionRowCommand selection_commands[] = {
        {"rows", -1, -1, {0}},
        {"beginswith", -1, -1, ""},
        {"contains", -1, -1, ""}
    };
    int selection_commands_parsing_result = get_selection_commands(args_count, arguments, selection_commands);
    if (selection_commands_parsing_result != 0)
        return selection_commands_parsing_result;

    /* COMMANDS COUNT */
    int edit_commands_count = 0;
    int processing_commands_count = 0;
    get_command_counts(args_count, arguments, &edit_commands_count, &processing_commands_count, command_definitions);

    /*  PREPARE TABLE EDITING COMMANDS  */
    Command edit_commands[edit_commands_count];
    int edit_commands_parsing_result = get_commands(args_count, arguments, command_definitions, edit_commands, 1);
    if (edit_commands_parsing_result)
        return selection_commands_parsing_result;

    /* PREPARE DATA PROCESSING COMMANDS */
    Command processing_commands[processing_commands_count];
    int processing_commands_parsing_result = get_commands(args_count, arguments, command_definitions, processing_commands, 2);
    if (processing_commands_parsing_result)
        return processing_commands_parsing_result;

    long row_index = -1;  // using long because max number of rows is not defined
    int column_count = 0;    // TODO: check if column count is valid in selection commands
    int original_column_count = column_count;

    char row_buffer[ROW_BUFFER_SIZE];

    while (fgets(row_buffer, ROW_BUFFER_SIZE, stdin))
    {
        row_index++;
        
        parse_line(row_buffer, cells_delimiter, row_index, !row_index ? &column_count : &original_column_count);
        if (!row_index)
            original_column_count = column_count;

        process_commands(row_buffer, edit_commands, edit_commands_count, cells_delimiter[0], row_index);
        process_commands(row_buffer, processing_commands, processing_commands_count, cells_delimiter[0], row_index);
        print_row(row_buffer);
    }
    return 0;
}
