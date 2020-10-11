//
// Created by Samuel Dobroň on 10. 10. 2020.
//
#include <stdio.h>
#include <string.h>


void get_cells_delimiter(char *delimiter_argument, char *raw_delimiter, char *delimiter)  // using delimiter_argument to check if not contains -d, in this case, delimiter is " "
{
    if (strcmp(delimiter_argument, "-d"))
    {
        strcpy(delimiter, " ");
        return;
    }

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
    return;
}


int main(int args_count, char *arguments[])
{
    (void)args_count;

    int delimiter_size = 1;
    if(!strcmp(arguments[1], "-d"))  // negation because of strcmp returns 0 in case, strings are the same
        delimiter_size = strlen(arguments[2]);

    char cells_delimiter[delimiter_size];
    get_cells_delimiter(arguments[1], arguments[2], cells_delimiter);

    printf("DELIMITER: '%s'\n", cells_delimiter);


    return 0;
}