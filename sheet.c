//
// Created by Samuel Dobroň on 10. 10. 2020.
//
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COLUMNS 103  // 103 because of maximum_row_size/maximum_cell_size = 102.5
#define CELL_SIZE 100

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

    char row_buffer[MAX_COLUMNS * CELL_SIZE];
    int row_buffer_position = 0;

    while ((character = getchar()))
    {
        if ((character == '\n' || character == '\r' || character == EOF) && row_buffer_position > 0)
        {
            row_buffer[row_buffer_position] = '\0';
            printf("%lu - %s\n", row_index, row_buffer);
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
