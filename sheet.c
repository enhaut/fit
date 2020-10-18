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

bool compare_strings(char *first, char *second)
{
    return strcmp(first, second) == 0 ? true : false;
}

void get_cells_delimiter(char *raw_delimiter, char *delimiter)  // using delimiter_argument to check if not contains -d, in this case, delimiter is " "
{
    int raw_delimiter_size = strlen(raw_delimiter);
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
    return;
}

bool is_defined_delimiter(int args_count, char **arguments)
{
    bool is_defined_delimiter = false;
    if (args_count >= 3 && compare_strings(arguments[1], "-d") && strlen(arguments[2]) > 0)
        is_defined_delimiter = true;

    return is_defined_delimiter;
}

void get_printable_delimiter(char *table_delimiter, char *printable_delimiter)
{
    printable_delimiter[0] = table_delimiter[0];
    printable_delimiter[1] = '\0';
    return;
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

int check_column_requirements(int column_size, int column_index, int column_count, int row_index, char *remaining_row)
{
    int return_code = 0;
    if (column_size > CELL_SIZE - 1)  // 1 bite is reserved for \0, -1 because of that
    {
        printf("Column is bigger than allowed!\n");
        return_code = -1;
    }else if (column_index > MAX_COLUMNS){
        printf("You are trying to use more columns than allowed!\n");
        return_code = -2;
    }else if (column_index + 1 != column_count && row_index > 0 && remaining_row == NULL){
        // +1 because column index is indexed from 0 and column_count from 1, checking of remaining_row to make sure that actual column is the last one
        printf("You have inconsistent column count!\n");
        return_code = -3;
    }
    return return_code;
}


int process_row(char *row, char *delimiter, int row_index, int *columns_count)
{
    int delimiter_size = strlen(delimiter);
    char *remaining_row = row;

    char columns[MAX_COLUMNS][CELL_SIZE] = {0};
    int column_index = 0;


    while (remaining_row != NULL)
    {
        int column_size = 0;
        int remaining_row_length = strlen(remaining_row);
        char original_row[remaining_row_length + 1];  // +1 for \0 at the end
        original_row[remaining_row_length] = '\0';
        strncpy(original_row, remaining_row, strlen(remaining_row));

        remaining_row = strstr(remaining_row, delimiter);  // at the end of row is no delimiter, strstr will return NULL so we cant calculate size of column
        if (remaining_row == NULL)
        {
            column_size = strlen(original_row);
        }else{
            column_size = strlen(original_row) - strlen(remaining_row);
            remaining_row += delimiter_size;  // move pointer behind the delimiter to force looking for delimiter, behind actual column
        }

        int column_requirements = check_column_requirements(column_size, column_index, *columns_count, row_index, remaining_row);
        if (column_requirements < 0)
            return column_requirements;

        if (column_size <= 0)
        {
            strcpy(columns[column_index], "");  // clear first \0 from array to mark column as used
            column_size = 1;
        }else{
            strncpy(columns[column_index], original_row, column_size);
        }
        columns[column_index][column_size] = '\0';
        column_index++;

    }

    if (row_index == 0)  // set column count from first row
        *columns_count = column_index;

    print_row(columns, delimiter, *columns_count);

    return 0;
}

// function for parsing selection commands using strings (contains, beginswith)
int process_string_selection_commands(char *arguments[], int *commands, int command_index, int saving_index)
{
    errno = 0;

    char *remaining_start;
    long start_at = strtol(arguments[command_index + 1], &remaining_start, 10);
    int should_contain_text_length = strlen(arguments[command_index + 2]);

    if (*remaining_start == '\0' && start_at > 0 &&
        0 < should_contain_text_length && should_contain_text_length < CELL_SIZE &&
        start_at != LONG_MIN && start_at != LONG_MAX && errno != ERANGE)
    {
        commands[saving_index] = command_index;
    }else{
        printf("Invalid syntax of command. Usage: beginswith/contains [from row] [should contains]\n");
        return 2;
    }
    return 0;
}

int get_selection_commands(int args_count, char *arguments[], int *commands)
{
    bool started_with_selection_commands = false;
    for (int command_index = 0; command_index < args_count;)  // using custom incrementing to skip command values
    {
        int saving_index = -1;  // dont need to make sure it could overflow, it is used in "if (saving_index)" only
        if (compare_strings(arguments[command_index], "rows"))
            saving_index = 0;
        else if (compare_strings(arguments[command_index], "beginswith"))
            saving_index = 1;
        else if (compare_strings(arguments[command_index], "contains"))
            saving_index = 2;

        if (saving_index == 0)
        {
            started_with_selection_commands = true;
            errno = 0;

            char *remaining_start;
            char *remaining_end;
            long starts_with = strtol(arguments[command_index + 1], &remaining_start, 10);
            long ends_with = strtol(arguments[command_index + 2], &remaining_end, 10);

            if ((compare_strings(remaining_start, "-") || (arguments[command_index + 1] != remaining_start && starts_with > 0 && *remaining_start == '\0')) && (starts_with != LONG_MIN && starts_with != LONG_MAX) && errno != ERANGE)
                if ((compare_strings(remaining_end, "-") || (arguments[command_index + 2] != remaining_end && ends_with > 0 && *remaining_end == '\0' && !compare_strings(remaining_start, "-") && ends_with > starts_with)) && (ends_with != LONG_MIN && ends_with != LONG_MAX) && errno != ERANGE)
                    commands[saving_index] = command_index;

            if (commands[saving_index] != command_index)
            {
                printf("Invalid syntax of row command! Usage: rows [from] [to]\n");
                return 1;
            }
            command_index += 2;  // 2 because there is 2 of values for rows command
        }else if(saving_index == 1 || saving_index == 2){
            int result = process_string_selection_commands(arguments, commands, command_index, saving_index);
            if (result > 0) {
                return result;
            }

            started_with_selection_commands = true;
            command_index += 2;
        }else if (started_with_selection_commands)
            break;  // prevent to parse not row selection commands, it is useless here

        command_index++;
    }
    return 0;
}

int main(int args_count, char *arguments[])
{
    int delimiter_size = 1;
    bool defined_custom_delimiter = is_defined_delimiter(args_count, arguments);
    if (defined_custom_delimiter)
        delimiter_size = strlen(arguments[2]);

    char cells_delimiter[delimiter_size + 1];  // +1 for null at the end
    if (defined_custom_delimiter)
        get_cells_delimiter(arguments[2], cells_delimiter);
    else
        strcpy(cells_delimiter, " ");

    int selection_commands_indexes[3] = {0};  // indexes of commands in *arguments[]
    int selection_commands_parsing_result = get_selection_commands(args_count, arguments, selection_commands_indexes);
    if (selection_commands_parsing_result != 0)
        return selection_commands_parsing_result;

    int character;
    unsigned long row_index = 0;  // using ulong because max number of rows is not defined
    int column_count = 0;

    char row_buffer[MAX_COLUMNS * CELL_SIZE];
    int row_buffer_position = 0;

    while ((character = getchar()))
    {
        if ((character == '\n' || character == '\r' || character == EOF) && row_buffer_position > 0)
        {
            row_buffer[row_buffer_position] = '\0';
            int result_of_processing = process_row(row_buffer, cells_delimiter, row_index, &column_count);
            if (result_of_processing != 0)
                return result_of_processing;
            row_index++;
            row_buffer_position = 0;
        }else{
            if (row_buffer_position == 10240)
            {
                printf("Row is too big!");
                return -3;
            }
            row_buffer[row_buffer_position] = character;
            row_buffer_position++;
        }

        if (character == EOF)
            break;
    }

    return 0;
}
