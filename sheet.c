//
// Created by Samuel Dobroň on 10. 10. 2020.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define MAX_COLUMNS 103  // 103 because of maximum_row_size/maximum_cell_size = 102.5
#define CELL_SIZE 100 + 1  // + 1 because we need to set \0 to the end
#define ROW_BUFFER_SIZE 10240 + 2    // +2 for \n and \0

// error codes
#define ERROR_BIGGER_COLUMN_THAN_ALLOWED 1
#define ERROR_MAXIMUM_COLUMN_LIMIT_REACHED 2
#define ERROR_INCONSISTENT_COLUMNS 3
#define ERROR_INVALID_COMMAND_USAGE 4
#define ERROR_MAXIMUM_ROW_SIZE_REACHED 5


struct SelectionRowCommand{
    char command[10 + 1];       // 10 is length of longest command, + 1 for \0
    long starting_row;          // -1 is unused command or invalid input, 0 is reserved for "rows" and means "-" in input
    long ending_row;            // used in "rows" command only, can contains value, -1 for invalid or unused, 0 means "-" in input
    char row_match[CELL_SIZE];  // used in "beginswith" and "contains" commands only
};


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

void get_printable_delimiter(const char *table_delimiter, char *printable_delimiter)
{
    printable_delimiter[0] = table_delimiter[0];
    printable_delimiter[1] = '\0';
}

void print_row(char parsed_row[MAX_COLUMNS][CELL_SIZE], char *delimiter, int columns_count)
{
    char delimiter_to_print[2];
    get_printable_delimiter(delimiter, delimiter_to_print);

    for (int column_index = 0; column_index < columns_count; column_index++)
    {
        printf("%s", parsed_row[column_index]);
        if (column_index < columns_count - 1)  // prevent adding delimiter to end of row
            printf("%s", delimiter);
    }

    printf("\n");

}

int check_column_requirements(int column_size, int column_index, int column_count, long row_index, const char *remaining_row)
{
    int return_code = 0;
    if (column_size > CELL_SIZE - 1)  // 1 bite is reserved for \0, -1 because of that
    {
        printf("Column is bigger than allowed!\n");
        return_code = ERROR_BIGGER_COLUMN_THAN_ALLOWED;
    }else if (column_index > MAX_COLUMNS){
        printf("You are trying to use more columns than allowed!\n");
        return_code = ERROR_MAXIMUM_COLUMN_LIMIT_REACHED;
    }else if (column_index + 1 != column_count && row_index > 0 && remaining_row == NULL){
        // +1 because column index is indexed from 0 and column_count from 1, checking of remaining_row to make sure that actual column is the last one
        printf("You have inconsistent column count!\n");
        return_code = ERROR_INCONSISTENT_COLUMNS;
    }
    return return_code;
}


bool can_process_row(struct SelectionRowCommand *selection_commands, long row_index, char parsed_row[MAX_COLUMNS][CELL_SIZE], bool last_row)
{
    bool can_process = true;

    for (int command_index = 0; command_index < 3; command_index++)
    {
        if (selection_commands[command_index].starting_row < 0)  // valid starting row is required in every command
            continue;

        if (compare_strings(selection_commands[command_index].command, "rows"))
        {
            if ((!selection_commands[command_index].starting_row && !selection_commands[command_index].ending_row && last_row) ||   // row indexes are "-"
                (selection_commands[command_index].starting_row && selection_commands[command_index].starting_row <= row_index &&   // checking starting row
                (!selection_commands[command_index].ending_row || selection_commands[command_index].ending_row >= row_index)))      // checking ending row
                can_process = true;
            else
                can_process = false;
        }else{
            char *remaining;
            remaining = strstr(parsed_row[selection_commands[command_index].starting_row - 1], selection_commands[command_index].row_match);  // -1 because parsed_row is indexing from 0

            if (remaining != NULL)
            {
                if (compare_strings(selection_commands[command_index].command, "contains") ||
                    (compare_strings(selection_commands[command_index].command, "beginswith") &&
                     strlen(remaining) == strlen(parsed_row[selection_commands[command_index].starting_row -1])))
                    can_process = can_process == true ? true : false;
                else
                    can_process = false;
            }else
                can_process = false;
        }
    }
    return can_process;
}

int process_row(char *row, char *delimiter, long row_index, int *columns_count, struct SelectionRowCommand *selection_commands, bool last_row)
{
    int delimiter_size = (int)strlen(delimiter);
    char *remaining_row = row;

    char columns[MAX_COLUMNS][CELL_SIZE] = {0};
    int column_index = 0;

    if (row_index &&  // let 0. row pass to set columns count
        selection_commands[0].starting_row > -1 &&
        selection_commands[1].starting_row < 0 &&
        selection_commands[2].starting_row < 0)  // in case, that is defined rows range only, we can check if row can be processed before processing
    {
        if (!can_process_row(selection_commands, row_index, columns, last_row))
            return 0;

    }

    while (remaining_row != NULL)
    {
        int column_size;  // value is initialized later
        int remaining_row_length = (int)strlen(remaining_row);
        char original_row[remaining_row_length + 1];  // +1 for \0 at the end
        original_row[remaining_row_length] = '\0';
        strncpy(original_row, remaining_row, strlen(remaining_row));

        remaining_row = strstr(remaining_row, delimiter);  // at the end of row is no delimiter, strstr will return NULL so we cant calculate size of column
        if (remaining_row == NULL)
            column_size = (int)strlen(original_row);
        else{
            column_size = (int)strlen(original_row) - (int)strlen(remaining_row);
            remaining_row += delimiter_size;  // move pointer behind the delimiter to force looking for delimiter, behind actual column
        }

        int column_requirements = check_column_requirements(column_size, column_index, *columns_count, row_index, remaining_row);
        if (column_requirements < 0)
            return column_requirements;

        if (column_size <= 0)
        {
            strcpy(columns[column_index], "");  // clear first \0 from array to mark column as used
            column_size = 1;
        }else
            strncpy(columns[column_index], original_row, column_size);

        columns[column_index][column_size] = '\0';
        column_index++;

    }

    if (row_index == 0)  // set column count from first row
        *columns_count = column_index;

    if (row_index && (selection_commands[1].starting_row || selection_commands[2].starting_row))
        if (!can_process_row(selection_commands, row_index, columns, last_row))
            return 0;

    print_row(columns, delimiter, *columns_count);

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
int process_string_selection_commands(struct SelectionRowCommand *command, char *row_match)
{
    int should_contain_text_length = (int)strlen(row_match);

    if (0 < should_contain_text_length && should_contain_text_length < CELL_SIZE)
        strcpy(command->row_match, row_match);
    else{
        fprintf(stderr, "Invalid syntax of command. Usage: beginswith/contains [from row] [should contains]\n");
        return ERROR_INVALID_COMMAND_USAGE;
    }
    return 0;
}

int get_selection_commands(int args_count, char *arguments[], struct SelectionRowCommand *commands)
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

int main(int args_count, char *arguments[])
{
    /* GET DELIMITER */
    int delimiter_size = 1;
    bool defined_custom_delimiter = is_defined_delimiter(args_count, arguments);
    if (defined_custom_delimiter)
        delimiter_size = (int)strlen(arguments[2]);

    char cells_delimiter[delimiter_size + 1];  // +1 for null at the end
    if (defined_custom_delimiter)
        get_cells_delimiter(arguments[2], cells_delimiter);
    else
        strcpy(cells_delimiter, " ");

    /* PREPARE SELECTION COMMANDS */
    struct SelectionRowCommand selection_commands[] = {
        {"rows", -1, -1, {0}},
        {"beginswith", -1, -1, ""},
        {"contains", -1, -1, ""}
    };
    int selection_commands_parsing_result = get_selection_commands(args_count, arguments, selection_commands);
    if (selection_commands_parsing_result != 0)
        return selection_commands_parsing_result;

    int character;
    long row_index = 0;  // using long because max number of rows is not defined
    int column_count = 0;    // TODO: check if column count is valid in selection commands
    bool last_row = false;

    char row_buffer[ROW_BUFFER_SIZE];
    int row_buffer_position = 0;

    while ((character = getchar()))
    {
        if ((character == '\n' || character == '\r' || character == EOF) && row_buffer_position > 0)
        {
            if (character == EOF)  // TODO: support last row detection at last used row
                last_row = true;
            row_buffer[row_buffer_position] = '\0';
            int result_of_processing = process_row(row_buffer, cells_delimiter, row_index, &column_count, selection_commands, last_row);
            if (result_of_processing != 0)
                return result_of_processing;
            row_index++;
            row_buffer_position = 0;
        }else{
            if (row_buffer_position == 10240)
            {
                printf("Row is too big!");
                return ERROR_MAXIMUM_ROW_SIZE_REACHED;
            }
            row_buffer[row_buffer_position] = (char)character;
            row_buffer_position++;
        }

        if (character == EOF)
            break;
    }

    return 0;
}
