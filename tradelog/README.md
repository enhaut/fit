# Tradelog

Create script for analysing trade logs files.

## Specification of interface
Usage:
```shell
tradelog [-h|--help] [FILTER] [COMMAND] [LOG [LOG ...]]
```
If no arguments are provided, the script just prints logs to `stdout`.

### Commands
Only one command per run is supported.
- `list-tick` - prints list of all the tickers
- `profit` - prints total profit
- `pos` - prints values of holding positions sorted by price
- `last-price` - prints the last known price for every ticker
- `hist-ord` - prints histogram of the count of transactions of ticker
    - in case `-w` argument is not provided, one transaction is represented by one `#` character,
      otherwise `-w` is the count of `#` characters in line with maximum value (`maximum value/2` is represented by `WIDTH/2` `#` characters)
- `graph-pos` - prints histogram version of `pos`
    - `-w` argument specifies maximum width graph. That means, line with maximum value is represented by 100% of `WIDTH`. In case `-w` is not provided, every character should represent the value of `1000`.
    - Positive values are represented by character `#` and negative values are represented by `!`.

### Filters
- `-a DATETIME` - filters transactions _after_ provided date `(YYYY-MM-DD HH:MM:SS)`
- `-b DATETIME` - filters transactions _before_ provided date `(YYYY-MM-DD HH:MM:SS)`
- `-t TICKER` - filters transactions for provided `TICKER`
- `-w WIDTH` - sets maximum width of generated graphs by `hist-ord` and `graph-pos` commands
    - only one `-w` argument in arguments is allowed, exit with error in case of multiple `-w` arguments

- `-h|--help` - prints help, then exits with exit code `0`


### Log files
The program should process logs compressed in `.gz` files also.
In case no files are provided in arguments, the program reads `stdin`.  
*Do not modify/delete provided files.*
#### Transactions
```
DATETIME;TICKER;TRANSACTION TYPE;UNIT PRICE;CURRENCY;VOLUME;ID
```
- Format of `DATETIME` is `YYYY-MM-DD HH:MM:SS`
- `TICKER` is name article
    - `TICKER` could not contains whitespace(s) or `;`
- `TRANSACTION TYPE` - type of transaction (`sell`/`buy`) decimal dot is represented as `.`
- `CURRENCY` - code of currency (`EUR`, `USD`, ...)
- `VOLUME` - represents the value of units in a transaction
- `ID`

## Rating
_not rated yet_
