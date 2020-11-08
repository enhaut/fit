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

#define MAX_COLUMNS 103  // 103 because of maximum_row_size/maximum_cell_size = 102.5
#define CELL_SIZE (100 + 1)  // + 1 because we need to set \0 to the end
#define ROW_BUFFER_SIZE (10240 + 2)    // +2 for \n and \0
#define COMMANDS_COUNT 28

// error codes
#define ERROR_BIGGER_COLUMN_THAN_ALLOWED 1
#define ERROR_MAXIMUM_COLUMN_LIMIT_REACHED 2
#define ERROR_INCONSISTENT_COLUMNS 3
#define ERROR_INVALID_COMMAND_USAGE 4
#define ERROR_MAXIMUM_ROW_SIZE_REACHED 5

#define USED_COLUMN_BITE 0x1F
#define DELETED_COLUMN_BITE 0x03


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

void get_cells_delimiter(char *raw_delimiter, char *delimiter)  // using delimiter_argument to check if not contains -d, in this case, delimiter is " "
{
    int raw_delimiter_size = (int)strlen(raw_delimiter);
    int free_position = 0;

    for(int i=0; i < raw_delimiter_size; i++)
    {
        if (strchr(delimiter, raw_delimiter[i]) == NULL)
        {
            delimiter[free_position] = raw_delimiter[i];
            free_position++;
        }
    }
    delimiter[free_position] = '\0';
}

bool is_defined_delimiter(int args_count, char **arguments)
{
    bool is_defined_delimiter = false;
    if (args_count >= 3 && compare_strings(arguments[1], "-d") && strlen(arguments[2]) > 0)
        is_defined_delimiter = true;

    return is_defined_delimiter;
}

void print_row(char parsed_row[][CELL_SIZE], char *delimiter)
{
    for (int column_index = 0; column_index < MAX_COLUMNS; column_index++)
    {
        if (parsed_row[column_index][CELL_SIZE -1] == DELETED_COLUMN_BITE) // ETX is used as mark of deleted column
            continue;

        /* 1F is hex number of unit separator it is used for marking column as used */
        if (parsed_row[column_index][0] == '\0' && parsed_row[column_index][CELL_SIZE - 1] != USED_COLUMN_BITE)
            break;

        /* make sure, delimiter wont print after deleting first (0.) column */
        if (column_index  && !(column_index == 1 && parsed_row[column_index - 1][CELL_SIZE - 1] == DELETED_COLUMN_BITE))
            printf("%c", delimiter[0]);

        printf("%s", parsed_row[column_index]);

    }
    printf("\n");
}

int check_column_requirements(size_t column_size, int column_index, int column_count, long row_index, const char *remaining_row)
{
    int return_code = 0;
    if (column_size > CELL_SIZE - 1)  // 1 bite is reserved for \0, -1 because of that
    {
        fprintf(stderr, "Column is bigger than allowed!\n");
        return_code = ERROR_BIGGER_COLUMN_THAN_ALLOWED;
    }else if (column_index > MAX_COLUMNS){
        fprintf(stderr, "You are trying to use more columns than allowed!\n");
        return_code = ERROR_MAXIMUM_COLUMN_LIMIT_REACHED;
    }else if (column_index + 1 != column_count && row_index > 0 && remaining_row == NULL){
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

int parse_line(char *raw_line, char parsed_line[][CELL_SIZE], char *delimiter, long row_index, int *columns_count)
{
    size_t delimiter_size = strlen(delimiter);
    char *remaining_row = raw_line;

    int column_index = 0;

    while (remaining_row != NULL)
    {
        size_t column_size;  // value is initialized later
        int remaining_row_length = (int)strlen(remaining_row);
        char original_row[remaining_row_length + 1];  // +1 for \0 at the end
        original_row[remaining_row_length] = '\0';
        strncpy(original_row, remaining_row, strlen(remaining_row));

        remaining_row = strstr(remaining_row, delimiter);
        if (remaining_row == NULL)  // at the end of row is no delimiter, strstr will return NULL so we cant calculate size of column
            column_size = strlen(original_row);
        else{
            column_size = strlen(original_row) - strlen(remaining_row);
            remaining_row += delimiter_size;  // move pointer behind the delimiter to force looking for delimiter behind actual column
        }

        int column_requirements = check_column_requirements(column_size, column_index, *columns_count, row_index, remaining_row);
        if (column_requirements > 0)
            return column_requirements;

        if (column_size <= 0)
        {
            parsed_line[column_index][CELL_SIZE - 1] = USED_COLUMN_BITE;  // using last unused bite to mark column as used, 1F is hex number of unit separator
            column_size = 1;
        }else
            strncpy(parsed_line[column_index], original_row, column_size);

        parsed_line[column_index][column_size] = '\0';
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

void remove_ending_newline_character(char *line)
{
    char *newline_character;
    if (line != NULL && (newline_character = strchr(line, '\n')) != NULL)
        *newline_character = '\0';
}

void acol(char row[][CELL_SIZE], CommandData *command_data)
{
    int last_column = 0;
    if (command_data->value > -1 && command_data->value <= MAX_COLUMNS)
        last_column = command_data->value;
    else
        for (; last_column < MAX_COLUMNS; last_column++)
            if (row[last_column][0] == '\0' && row[last_column][CELL_SIZE - 1] != USED_COLUMN_BITE)
                break;

    row[last_column][CELL_SIZE - 1] = USED_COLUMN_BITE;
    command_data->value = last_column;
}

void dcol(char row[][CELL_SIZE], CommandData *command_data)
{
    row[command_data->start][CELL_SIZE - 1] = DELETED_COLUMN_BITE;  // 03 is hex of ETX, used as mark column as deleted
}

void dcols(char row[][CELL_SIZE], CommandData *command_data)    // TODO: chova sa to inak pri ponechani 1. stlpca a pri ponechani posledneho
{
    CommandData changing_command_data = *command_data;
    for (long column = command_data->start; column <= command_data->end; column++) {
        dcol(row, &changing_command_data);
        changing_command_data.start++;
    }
}

void icol(char row[][CELL_SIZE], CommandData *command_data)
{
    long column = command_data->start;
    memmove(row[column + 1], row[column], CELL_SIZE * MAX_COLUMNS - (column + 1) * CELL_SIZE);
    row[column][0] = '\0';
    row[column][CELL_SIZE - 1] = USED_COLUMN_BITE;  // mark column as used, but empty
}

void empty_function(char row[][CELL_SIZE], CommandData *command_data)   // function used to set some function to command so mark command as used and should be performed
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
        if (command_index + command_arguments > args_count) //ERROR DO MATKY PICI VACEJ ARGUMENTOV JAK MOZE KKT SPROSTY
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

void process_commands(char row[][CELL_SIZE], Command *edit_commands, int edit_commands_count)
{
    for (int command_index = 0; command_index < edit_commands_count; command_index++)
    {
        function_ptr edit_function = edit_commands[command_index].processing_function;
        edit_function(row, &edit_commands[command_index].data);
    }
}

void column_tolower(char row[][CELL_SIZE], CommandData *command)
{
    char *column = row[command->start];
    for (size_t character_index = 0; character_index < strlen(column); character_index++)
        column[character_index] = (char)tolower(column[character_index]);
}

void column_toupper(char row[][CELL_SIZE], CommandData *command)
{
    char *column = row[command->start];
    for (size_t character_index = 0; character_index < strlen(column); character_index++)
        column[character_index] = (char)toupper(column[character_index]);
}

void column_round(char row[][CELL_SIZE], CommandData *command)
{
    char *column = row[command->start];
    double to_round;
    char *remaining = NULL;
    errno = 0;

    to_round = strtod(column, &remaining);
    if (errno != 0 || *remaining != '\0')
        return;

    int rounded = (int)(to_round + (to_round > 0.0 ? 0.5 : -0.5));
    sprintf(column, "%d", rounded);
}

void column_int(char row[][CELL_SIZE], CommandData *command)
{
    char *column = row[command->start];
    char *decimal_delimiter_position = NULL;
    decimal_delimiter_position = strchr(column, *".");
    if (decimal_delimiter_position == NULL)
        return;
    size_t dot_position = strlen(column) - strlen(decimal_delimiter_position);
    column[dot_position] = '\0';
}

void copy(char row[][CELL_SIZE], CommandData *command)
{
    strcpy(row[command->end], row[command->start]);
}

void swap(char row[][CELL_SIZE], CommandData *command)
{
    char *what = row[command->start];
    char *with = row[command->end];
    char temp[CELL_SIZE];
    strcpy(temp, what);
    strcpy(what, with);
    strcpy(with, temp);

    /* MARK EMPTY COLUMNS AS USED */
    if (!strlen(what))
        what[CELL_SIZE - 1] = USED_COLUMN_BITE;
    if (!strlen(with))
        with[CELL_SIZE - 1] = USED_COLUMN_BITE;
}

void move(char row[][CELL_SIZE], CommandData *command)
{
    CommandData swapped_directions;         // icol is used with swapped directions, so i need to swap it
    swapped_directions.start = command->end;
    swapped_directions.end = command->start;

    icol(row, &swapped_directions);                             // icol will move columns to right, to make space for new one
    strcpy(row[command->end], row[command->start + 1]);     // +1 because icol moved that column to the right
    row[command->start+1][CELL_SIZE - 1] = DELETED_COLUMN_BITE; // +1 for same reason
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

void csum(char row[][CELL_SIZE], CommandData *command)
{
    float sum = 0;
    float number_to_add = 0;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        bool valid_number = get_numeric_cell_value(row[column], &number_to_add);
        if (!valid_number)
            continue;
        sum += number_to_add;
    }
    sprintf(row[command->start], "%g", sum);
}

void cavg(char row[][CELL_SIZE], CommandData *command)
{
    float sum = 0;
    float valid_columns = 0;
    float number_to_add;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        bool valid_number = get_numeric_cell_value(row[column], &number_to_add);
        if (!valid_number)
            continue;
        sum += number_to_add;
        valid_columns++;
    }
    valid_columns = valid_columns > 0 ? valid_columns : 1;  // set 1 to avoid divide by zero
    sprintf(row[command->start], "%g", (sum/valid_columns));
}

void cmin(char row[][CELL_SIZE], CommandData *command)
{
    float min = 0;
    float cell_value;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        bool valid_number = get_numeric_cell_value(row[column], &cell_value);
        if (!valid_number)
            continue;
        if (cell_value < min || column == command->end)    // first iteration, initialized 0 have to be rewrited
            min = cell_value;
    }
    sprintf(row[command->start], "%g", min);
}
void cmax(char row[][CELL_SIZE], CommandData *command)
{
    float max = 0;
    float cell_value;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        bool valid_number = get_numeric_cell_value(row[column], &cell_value);
        if (!valid_number)
            continue;
        if (cell_value > max || column == command->end)   // first iteration, initialized 0 have to be rewrited
            max = cell_value;
    }
    sprintf(row[command->start], "%g", max);
}

void ccount(char row[][CELL_SIZE], CommandData *command)  // TODO: tu ma byt asi pocet stlpcov, ktore obsahuju cislo
{
    int filled_cells = 0;

    for (int column = (int)command->end; column <= command->value; column++)
    {
        if (strlen(row[column]) == 0)
            continue;
        filled_cells++;
    }
    sprintf(row[command->start], "%d", filled_cells);
}

void cseq(char row[][CELL_SIZE], CommandData *command)
{
    int seq_start = command->value + 1; // +1 because parsing decreased it by 1
    int starting_column = (int)command->start;
    for (int column = starting_column; column <= command->end; column++)
        sprintf(row[column], "%d", seq_start+(column-starting_column)); // add # of iteration to seq_start, to increase it
}

void cset(char row[][CELL_SIZE], CommandData *command)
{
    strcpy(row[command->start], command->text_value);
}

/* implementation od drow and drows commands */
void drow_s(long row_index, CommandData *command_data, bool *should_delete)
{
    if ((row_index == command_data->start && command_data->end == -1) ||        // drow command
        (row_index >= command_data->start && row_index <= command_data->end))   // drows command
        *should_delete = true;
}

bool delete_rows_by_index(Command *edit_commands, int edit_commands_count, long row_index)
{
    for (int command_index = 0; command_index < edit_commands_count; command_index++)
    {
        if (edit_commands[command_index].processing_function != drow_s)
            continue;

        bool should_delete = false;
        drow_s(row_index - 1, &edit_commands[command_index].data, &should_delete);   // -1 because rows are indexed from 1 and commands from 0

        if (should_delete)
            return true;
    }
    return false;
}

void get_all_commands(CommandDefinition *commands)
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
            {"round",   1, 2, column_round},
            {"int",     1, 2, column_int},
            {"copy",    2, 2, copy},
            {"swap",    2, 2, swap},
            {"move",    2, 2, move},
            {"cavg",    3, 2, cavg},
            {"csum",    3, 2, csum},
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

int main(int args_count, char *arguments[])
{
    /* GET DELIMITER */
    size_t delimiter_size = 1;
    bool defined_custom_delimiter = is_defined_delimiter(args_count, arguments);
    if (defined_custom_delimiter)
        delimiter_size = strlen(arguments[2]);

    char cells_delimiter[delimiter_size + 1];  // +1 for null at the end
    if (defined_custom_delimiter)
        get_cells_delimiter(arguments[2], cells_delimiter);
    else
        strcpy(cells_delimiter, " ");

    CommandDefinition command_definitions[COMMANDS_COUNT];
    get_all_commands(command_definitions);

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
    bool last_row = false;

    char row_buffer[ROW_BUFFER_SIZE];
    char next_row_buffer[ROW_BUFFER_SIZE];  // loading 2 lines to detect that next line is only \nEOF
    char *reading_result; // result of reading

    reading_result = fgets(next_row_buffer, ROW_BUFFER_SIZE, stdin);  // load first line

    while (reading_result != NULL)
    {
        strcpy(row_buffer, next_row_buffer);  // TODO: optimalization - switching pointers of row buffer and next_row_buffer
        reading_result = fgets(next_row_buffer, ROW_BUFFER_SIZE, stdin);
        if (reading_result == NULL)
            last_row = true;

        char columns[MAX_COLUMNS][CELL_SIZE] = {0};
        row_index++;

        /* in case, that is defined "rows" range only, we can check if row can be processed before line processing */
        if (row_index &&  // let 0. row pass to set columns count
            selection_commands[0].starting_row > -1 &&
            selection_commands[1].starting_row < 0 &&
            selection_commands[2].starting_row < 0)
        {
            if (!can_process_row(selection_commands, row_index, columns, last_row)) {
                continue;
            }
        }

        if (row_index && delete_rows_by_index(edit_commands, edit_commands_count, row_index))
            continue;

        remove_ending_newline_character(row_buffer);
        parse_line(row_buffer, columns, cells_delimiter, row_index, !row_index ? &column_count : &original_column_count);
        if (!row_index)
            original_column_count = column_count;

        if (row_index && (selection_commands[1].starting_row || selection_commands[2].starting_row))
            if (!can_process_row(selection_commands, row_index, columns, last_row))
                continue;

        process_commands(columns, edit_commands, edit_commands_count);
        process_commands(columns, processing_commands, processing_commands_count);
        print_row(columns, cells_delimiter);   // TODO: overit pri printe ci ten stlpec vobec nieco obsahuje
    }
    return 0;
}
