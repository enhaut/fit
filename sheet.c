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

bool defined_delimiter(int args_count, char *arguments[])
{
    bool is_defined_delimiter = false;
    if (args_count >= 3 && strcmp(arguments[1], "-d") == 0)
        is_defined_delimiter = true;

    return is_defined_delimiter;
}


int process_row(char *row, char *delimiter, int row_index, int *columns_count)
{
    (void)row_index;
    (void)columns_count;
    int delimiter_size = strlen(delimiter);
    char *remaining_row = row;

    char column[MAX_COLUMNS][CELL_SIZE] = {0};
    int column_index = 0;


    while (remaining_row != NULL)
    {
        int column_size = 0;
        char original_row[strlen(remaining_row) + 1];
        original_row[strlen(remaining_row)] = '\0';
        strncpy(original_row, remaining_row, strlen(remaining_row));

        remaining_row = strstr(remaining_row, delimiter);  // at the end of row is no delimiter, strstr will return NULL so we cant calculate size of column
        if (remaining_row == NULL)
        {
            column_size = strlen(original_row);
        }else{
            column_size = strlen(original_row) - strlen(remaining_row);
            remaining_row += delimiter_size;  // move pointer behind the delimiter to force looking for delimiter, behind actual column
        }

        if (column_size <= 0)
        {
            strcpy(column[column_index], "");
            column_size = 1;
        }else{
            strncpy(column[column_index], original_row, column_size);
            printf("%s%s", column[column_index], delimiter);
        }
        column[column_index][column_size] = '\0';
        column_index++;

    }
    printf("\n");

    return 1;

}

int main(int args_count, char *arguments[])
{
    int delimiter_size = 1;
    bool defined_custom_delimiter = defined_delimiter(args_count, arguments);
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
            process_row(row_buffer, cells_delimiter, row_index, &column_count);
            row_index++;
            row_buffer_position = 0;
        }else{
            row_buffer[row_buffer_position] = character;
            row_buffer_position++;
        }

        if (character == EOF)
            break;
    }

    return 0;
}
