//
// Created by Samuel Dobroň on 10. 10. 2020.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COLUMNS 103  // 103 because of maximum_row_size/maximum_cell_size = 102.5
#define CELL_SIZE 100 + 1  // + 1 because we need to set \0 to the end

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
    if (args_count >= 3 && strcmp(arguments[1], "-d") == 0)
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
