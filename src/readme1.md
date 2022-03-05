Documentation of Project Implementation for IPP 2021/2022  
Name and surname: Samuel Dobron  
Login: xdobro23  

# Usage
Script parses instructions from `stdin`.  
It can be started using following command:
```bash
$ php parser.php < program.src
```
Use `--help` argument for more information
regards to usage.
### PHP version
Script was developed and tested for `PHP 8.1.3`.

### Exit codes
- `0` on success
- `21` invalid program header
- `22` invalid instruction
- `23` invalid instruction call parameter(s)

# Implementation details
For whole project has been used [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) standards and concepts.
Parser parses instructions from `stdin`, does lexical and
syntax analysis, translated instructions are then printed
to the `stdout`, `stderr` is used for error and debugging
information.

## Parsing an arguments
For program arguments parsing is responsible 
`ArgumentParser` class. It needs to be initialized with
`$args` and `$argc` parameters - count and list of program
arguments.   
It has one and simple entry point - `parse()` method, which
does all the stuff behind - checks number of arguments,
and it's validity.  
Only one argument is supported:
- `--help` prints usage information

## Instructions
Internal representation of instructions are classes,
based on instruction's number of parameters:
- `NoArgsInstruction` - represent instructions without parameters
- `SingleArgsInstruction` - represents instructions with one parameter
- `DoubleArgsInstruction` - represents instructions with 2 parameters
- `TripleArgsInstruction` - represents instructions with 3 parameters

Classes above are children of `Instruction` class. I decided
to implement it using [Factory Method](https://stackoverflow.com/a/50015395) 
design pattern. `Instruction` class is a "factory" class
which, is a parent for all the instructions classes, whose
are initialized in `InstructionParser::get_instruction()`
method.  
`Instruction` class implements all the common methods 
used while translating instructions. Methods with 
specific implementation are then overloaded in children
classes. For example - `Instruction::get_arguments()` method
```php
public function get_arguments($xml_dom, $node)
{
    throw new Exception("Not implemented");
}
```
Method above has different implementation for all the 
instruction classes. So it has to be overloaded and 
implemented there.

## Parsing an instructions
Lines of instructions are read from `stdin`, then parsed by 
`Parser::parse_lines()` method. Which removes all the unnecessary
characters around instructions.  
Lexical and syntax analysis
is proceeded by regular expressions matching.  
Regular expressions templates are saved in (`InstructionParser::instructions`)
array of arrays based on number of instructions parameters.

### Building an instruction regexp
Instruction's regular expressions' ale built dynamically, 
name and parameters are saved in `InstructionParser::instructions`
array.
```php
const instructions = array(
    1 => array("(DEFVAR)", self::insParamsRegexp["var"]), ... ),
);
```
First element of array is name, the next ones are references
to regular expression to required instruction parameter.  
Following parameter types are supported:
- [`<var>`](https://www.debuggex.com/r/q8gpK0HPyHcVQA5d) - variable
- [`<label>`](https://www.debuggex.com/r/7Ijb5j0-7kOVE27L) - label for jumps
- [`<symb>`](https://www.debuggex.com/r/pfmHeiUG1lyDYk2U) - variable/constant
- [`<type>`](https://www.debuggex.com/r/SFjQ5uVpG6OhSGBM) - type

Regular expressions used for building instruction's
regular expression are constants at the start of `InstructionParser`
class.
## Optimizations
Instructions are stored in [doubly linked list](https://www.php.net/manual/en/class.spldoublylinkedlist.php)
to make future implementation of code optimizations easier.
