Documentation of Project Implementation for IPP 2021/2022  
Name and surname: Samuel Dobron  
Login: xdobro23  

# Interpret
Interpret for `IPPcode22` language.
## Usage
Script parses instructions from file defined by `--source` argument or, if no
`--source` argument was provided, it reads `stdin`. But in case of reading
instructions from `stdin` the `--input` argument needs to be set (see [arguments](#parsing-an-arguments)).
```bash
$ python interpret.py --source="program.xml"
```
#### Python version
Script was developed and tested for `Python 3.8.12`.

#### Exit codes
- `0` on success
- `31` invalid XML syntax of source
- `32` invalid XML structure of source eg. `<arg*>` tag outside of `<instruction>` tag
- `52` invalid semantics (undefined label, variable redefinition)
- `53` invalid operand types
- `54` trying to access undefined variable
- `55` frame does not exists
- `56` missing value at stack
- `57` invalid operand value (division by zero, ...)
- `58` invalid operation with string

## Implementation details
Implementation is pretty straight-forward. There is class `Interpret` which is responsible for
parsing instruction from source `XML` file. Based on `opcode` attribute value it detects instruction
that should be interpreted.  
Every instruction has a separate class for example, for interpreting instruction `MOVE` is 
responsible class named `InstructionMOVE`. All the instruction classes have a common parent - `Instruction` class.  
The `Instruction::__init__()` method also loads the arguments and its values (see [`ArumentType`](#argumenttype)) from raw `XML` which is passed 
to the `Instruction` constructor method.


### `Instruction`
This class has to be the parent for all the instruction classes because Abstract Factory principle
was used. The product of factory is `InstructionXXX`, where `XXX` is name of instruction - `InstructionREAD`.
It is responsible for checking 
validity of arguments, number of arguments, it's types, ...  
`Instruction::__get_instruction_class()` method is responsible for choosing right instruction
class based on `opcode` attribute value. The returned value is not the initialized instance
of instruction class but pointer to the class instead. The initialization is then performed 
in `Instruction::parse_instructions()` method which calls all the parent's `__init__()` methods.
  
The most important method that `Instruction` has is `interpret()`, calling this method should
perform the action based on instruction name. This method is implemented in inherited child classes.  
`Instruction` are connected using doubly linked list, previous and next instructions are
accessible using properties `prev` and `next`. `next` property and instruction pointer
principle are then used to determine next instruction to interpret.

### `Variable`
Class `Variable` is `dataclass` of variables. Getting and setting values are implemented
using built-in `__setattr__()` and `__getattribute__()` methods. It provides better control
of handing variable initialization and types.  
Whenever setting the value `__setattr__()` method sets property `initialized` as `True`,
_conditional checking of variable initialization and setting variable as an initialized 
would be slower_. Similar principle is used for setting the type of variable.  
For getting the value is modified `__getattribute__()` method, it checks if variable is 
initialized if not, error code `56` is returned.

### `MemoryFrame`
`MemoryFrame` implements storing variables. There are 3 memory frames:
- Global frame - `GF`
- Temporary frame - `TF`
- Local frame - `LF`  
These frames are defined in `Interpret::frames` dictionary.

### `ArumentType`
This class is abstraction of instruction argument - `MOVE GF@to int@5` this instruction call
has 2 arguments - `GF@to` and `int@5`. Currently, interpret supports these arguments:
- `ConstatArgument` - handles constants: `int@5`, `string@aaa`
- `VariableArgument`- handles variables: `GF@x`, `LF@y`, `TF@z`
- `LabelArgument` - handles labels
- `TypeArgument` - handles types: `int`, `string` and `bool`  

`VariableArgument` does not hold the value of variable, `Variable` class does.

### Parsing an arguments
Arguments parsing is not implemented absolutely from scratch. Python's standard library
`argparse` and it's `ArgParse` class are used. These arguments are supported:
- `--help` - prints help
- `--source={file}` - `XML` source of program
- `--input={file}` - file used for reading the inputs  

At least one of `--source` or `--input` needs to be set. The remaining
one uses `stdin`.
