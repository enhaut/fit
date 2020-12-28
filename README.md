# Advanced table processor
The aim of the project is to create an advanced table processor. You should use dynamic memory allocation.
Application has to process table data in the file provided via command line argument. Actions will be in command line arguments too. 

# Compile
Use makefile. That is the easiest way:

    make
Alternatively, you can compile it on your own:
 
    gcc -std=c99 -Wall -Wextra -Werror -pedantic -c -o sps.o sps.c

# Usage

    ./sheet [-d DELIMITER] CMD_SEQUENCE FILE

## Requirements
#### For delimiter
Argument `-d` defines what characters could be interpreted as column delimiters.
In case delimiter is not set, the delimiter is space ` `.  
The first delimiter from delimiters will be used as a delimiter for output data.

#### For content of table
CMD_SEQUENCE could be defined in the file via command `-cCMD_SEQUENCE_FILE`
The table could contain escaped strings. Escaped could be whole cell, substring of cell, or
just one char using `"` or `\` for one character.

## Available commands
Command will be processed for every cell in selected range. The range is defined by selection commands.
By default, first cell of the first row is selected.

### Selection commands
- `[R,C]` - selects row `R` and column `C`
- `[R,_]` - selects whole row `R`
- `[_,C]` - selects column `C`
- `[R1,C1,R2,C2]` - selects window of cells starting at row `R1` and column `C1` and ends at `R2` and column `C2`
- `[_,_]` - selects whole table
- `[min]` - selects cell with lowest numeric value
- `[max]` - selects cell with highest numeric value
- `[find STR]` - selects cell that contains (sub)string `STR`
- `[_]` - selection from temporary value will be set as actual selection 

### Table editing commands
Commands that change the table dimension.
- `irow` - adds new row with empty cells above selected range
- `arow` - adds new row with empty cells bellow selected range
- `drow` - deletes selected rows
- `icol` - adds empty column before selected range
- `acol` - adds empty column behind selected range
- `dcol C` - deletes selected columns

### Required commands:
#### Table edit commands
- `set STR` - sets string `STR` to all cells in selected range 
- `clear` - wipes content of all cells in selected range 
- `swap [R,C]` - swaps selected cell with cell at `R` row and `C` column  
- `sum [R,C]` - sum of numeric values of cells in selected range will be set to cell at `R` row and `C` column  
- `avg [R,C]` - average of numeric values of cells in selected range will be set to cell at `R` row and `C` column  
- `count [R,C]` - counts non empty cells in selected range will be set to cell at `R` row and `C` column  
- `len [R,C]` - sets lenght of selected cell to cell at `R` row and `C` column

#### Temporary variables commands
Table processor supports 10 user defined temporary variables identified as `_0` - `0_9`.
Variables are empty by default. There are 11. temporary variable `[_]` that can store selection range.
- `def _X` - the value of selected cell will be set to temporary value `_X`
- `use _X` - the value of temporary variable `_X` will be set to the selected cell
- `inc _X` - the numerical value of a temporary variable `_X` will be increased by 1. In case, variable `_X` is not numeric, it should be set to 1.
- `[set]` - sets an actual range of selected cells to special temporary variable `_`

### Optional commands:
#### Sequence control commands
Not implemented.
- `goto +N` - skips `N` commands
- `goto -N` - skips `-N` commands
- `iszero _X ±N` - in case the value of temporary value `_X` is zero, skips `±N` commands
- `sub _X _Y` - the value of temporary variable `_X` will be decreased by the value of temporary value `_Y`

### Rating of my project 
14/14 + 0.19 premium points
