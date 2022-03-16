import argparse
import sys
from dataclasses import dataclass
from typing import Union, List, Dict
import xml.etree.ElementTree as ET
import re


def error_exit(message: str, code: int):
    print(message, file=sys.stderr)
    sys.exit(code)


class MemoryFrame:
    @dataclass
    class Variable:
        name: str
        value: Union[str, int, float, bool, None] = None
        initialized: bool = False
        var_type: classmethod = None

        def __setattr__(self, key, value):
            if key == "value":
                self.var_type = type(value)  # set variable type based on its value

            super().__setattr__(key, value)

    def __init__(self, initialized=False, parent="MemoryFrame"):
        self._initialized = initialized
        self._variables: List["MemoryFrame.Variable"] = []
        self._stack: List[Union[str, int, float, bool, None]] = []
        self._parent = parent

    def __repr__(self):
        ret = ""
        for variable in self._variables:
            ret += f"{variable.name}/{variable.var_type}({variable.initialized})={variable.value};"

        ret += "__STACK__:"
        for value in self._stack:
            ret += f"{value};"

        return ret

    def get_variable(self, name: str):
        for variable in self._variables:
            if variable.name == name:
                return variable

        return None  # undefined variable

    def set_variable(self, name: str):
        self._variables.append(MemoryFrame.Variable(name))

    def get_stack_var(self):
        if not self._stack:
            error_exit("Stack is empty", 56)

        return self._stack.pop()

    def set_stack_var(self, value: Union[str, int, float, bool, None]):
        self._stack.append(value)


class ArgumentType:
    def __init__(self, name: str, element: ET.Element):
        self.name = name
        self.value = None
        self.raw_element = element

        self.set_value()

    def __repr__(self):
        return f"{self.__class__.__name__} - {self.name}: {self.value}"

    def set_value(self):
        raise NotImplementedError


class VariableArgument(ArgumentType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.frame = self.get_frame()
        self.type = self.get_type()

    def set_value(self):
        self.name = self.raw_element.text
        self.value = self.raw_element.text  # TODO: just hotfix, probably setting name is enough, DEFVAR needs to refactor

    def get_frame(self):
        pass

    def get_type(self):
        pass


class ConstantArgument(ArgumentType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.name = None
        self.type = self.get_type()

    def set_value(self):
        self.value = self.raw_element.text

    def get_type(self):
        raw_type = self.raw_element.attrib["type"]
        var_type = type(None)
        if raw_type == "int":
            var_type = type(1)
        elif raw_type == "string":
            var_type = type("")
        elif raw_type == "bool":
            var_type = type(True)
        elif raw_type == "nil":
            var_type = type(None)
        else:
            error_exit("Unsupported variable type", 52)

        return var_type


class LabelArgument(ArgumentType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def set_value(self):
        self.value = self.raw_element.text


class TypeArgument(ArgumentType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def set_value(self):
        self.value = self.raw_element.text


class Instruction:
    __VAR_NAME_REGEXP = r"[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*"
    __CONSTANTS_REGEXP = r"(?:(?:0[xX][0-9a-fA-F]+|[+-]?[0-9]+))|(?:true|false)|nil|(?:[^#\\\\\s]|\\\\\d{3})*"
    VAR_REGEXPS = {
        "var": "([LTG]F@" + __VAR_NAME_REGEXP + ")",
        "label": "(" + __VAR_NAME_REGEXP + ")",
        "symb": "((?:[LTG]F@" + __VAR_NAME_REGEXP + ")|(?:" + __CONSTANTS_REGEXP + "))",
        "type": "(int|string|bool)"
    }
    INSTRUCTIONS = [
        [("CREATEFRAME",), ("PUSHFRAME",), ("POPFRAME",), ("RETURN",), ("BREAK",)],
        [
            ("DEFVAR", VAR_REGEXPS["var"]),
            ("CALL", VAR_REGEXPS["label"]),
            ("PUSHS", VAR_REGEXPS["symb"]),
            ("POPS", VAR_REGEXPS["var"]),
            ("WRITE", VAR_REGEXPS["symb"]),
            ("LABEL", VAR_REGEXPS["label"]),
            ("JUMP", VAR_REGEXPS["label"]),
            ("EXIT", VAR_REGEXPS["symb"]),
            ("DPRINT", VAR_REGEXPS["symb"])
        ],
        [
            ("MOVE", VAR_REGEXPS["var"], VAR_REGEXPS["symb"]),
            ("INT2CHAR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"]),
            ("READ", VAR_REGEXPS["var"], VAR_REGEXPS["type"]),
            ("NOT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"]),
            ("STRLEN", VAR_REGEXPS["var"], VAR_REGEXPS["symb"]),
            ("TYPE", VAR_REGEXPS["var"], VAR_REGEXPS["symb"])
        ],
        [
            ("ADD", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("SUB", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("MUL", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("IDIV", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("LT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("GT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("EQ", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("AND", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("OR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("STRI2INT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("CONCAT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("GETCHAR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("SETCHAR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("JUMPIFEQ", VAR_REGEXPS["label"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("JUMPIFNEQ", VAR_REGEXPS["label"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"])
        ]
    ]

    def __init__(self, name: str, xml_raw: ET.ElementTree):
        self.name = name.upper()
        self.raw_instruction = xml_raw
        self.ins_regexp = self.get_instruction_regexp()

        self.arg1: ArgumentType = None
        self.arg2: ArgumentType = None
        self.arg3: ArgumentType = None
        self.set_arguments()

    def __repr__(self):
        ret = f"{self.__class__.__name__} - {self.name} \n"
        ret += f"\t{self.arg1}\n"
        ret += f"\t{self.arg2}\n"
        ret += f"\t{self.arg3}"
        return ret

    @staticmethod
    def check_parameter_attributes(parameter: ET.Element):
        if parameter is not None and "type" in parameter.keys() and len(parameter.items()) == 1:
            return True
        error_exit("Invalid param attributes", 31)

    def __get_instruction_array_index(self):
        if isinstance(self, NoArgsInstruction):
            return 0
        elif isinstance(self, SingleArgsInstruction):
            return 1
        elif isinstance(self, DoubleArgsInstruction):
            return 2
        elif isinstance(self, TripleArgsInstruction):
            return 3
        else:
            raise NotImplementedError("Instruction with >3 arguments is not (yet) supported")

    def get_instruction_regexp(self):
        instructions_array = self.INSTRUCTIONS[self.__get_instruction_array_index()]

        for instruction_array in instructions_array:
            if instruction_array[0] != self.name:
                continue
            return instruction_array

        error_exit(f"Invalid instruction: {self.name}", 32)

    def set_arguments(self):
        raise NotImplementedError

    def check_argument(self, attr_id: int, attribute_element: ET.Element):
        attr_id += 1  # first member of array is instruction name

        if not re.match(self.ins_regexp[attr_id], attribute_element.text, re.UNICODE):
            error_exit("Invalid attribute type!", 53)

    def get_argument(self, element: ET.Element) -> ArgumentType:
        if element.attrib["type"] == "label":
            arg_class = LabelArgument
        elif element.attrib["type"] == "var":
            arg_class = VariableArgument
        elif element.attrib["type"] == "int" or element.attrib["type"] == "bool" or \
                element.attrib["type"] == "string" or element.attrib["type"] == "nil":
            arg_class = ConstantArgument
        elif element.attrib["type"] == "type":
            arg_class = TypeArgument
        else:
            raise NotImplementedError("Other attribute types are not (yet) implemented!")

        return arg_class(element.attrib["type"], element)  # TODO: check if "type" attribute is present

    def _get_frame_from_var_name(self, name: str = None):
        if not name:
            name = self.arg1.value

        if name.startswith("TF@"):
            return "TF"
        elif name.startswith("LF@"):
            return "LF"
        elif name.startswith("GF"):
            return "GF"

        error_exit(f"Invalid frame name: {name}", 55)

    def _get_variable(self, name: str, memory: Dict[str, List[MemoryFrame]]):
        frame = self._get_frame_from_var_name(name)
        if not memory[frame]:
            error_exit(f"Frame {frame} is not initialized", 55)

        variable = memory[frame][0].get_variable(name[3:])
        if variable is None:
            error_exit(f"Undefined variable {name}", 54)

        return variable

    @staticmethod
    def __get_value_from_constant(const: ConstantArgument):
        if const.type == int:
            try:
                return int(const.value)  # TODO: implement another basis
            except ValueError:
                error_exit("Invalid number", 52)
        elif const.type == bool:
            if const.value == "false":
                return False
            else:
                return True  # TODO: check if everything != false is evaluated as a true
        elif const.type == str:
            return const.value
        elif const.type == type(None):
            return None

        error_exit("Unsupported constant value", 52)

    def __get_value_from_var(self, var: VariableArgument, memory):
        variable = self._get_variable(var.name, memory)
        return variable.value

    def _get_value_from_symb(self, symb: Union[ConstantArgument, VariableArgument, ArgumentType], memory):
        if isinstance(symb, ConstantArgument):
            return self.__get_value_from_constant(symb)
        else:
            return self.__get_value_from_var(symb, memory)

    @staticmethod
    def set_value(variable: "MemoryFrame.Variable", symb_value: Union[bool, int, str, None]):
        if variable.initialized and variable.var_type != type(symb_value):
            error_exit(f"Incompatible types of variables: {variable.name} <- {symb_value}", 53)

        variable.value = symb_value
        variable.initialized = True

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        raise NotImplementedError()


class NoArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        pass

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        super().interpret(memory)


class InstructionCREATEFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        memory["TF"] = [MemoryFrame(True, None)]


class InstructionPUSHFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        if not memory["TF"]:
            error_exit("No frame to push!", 55)

        memory["LF"].append(memory["TF"][0])
        memory["TF"] = []


class InstructionPOPFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        if not memory["LF"]:
            error_exit("No frame to pop!", 55)

        memory["TF"] = [memory["LF"].pop()]


class SingleArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        arg = self.raw_instruction.find("arg1")
        self.check_parameter_attributes(arg)

        self.check_argument(0, arg)
        self.arg1 = self.get_argument(arg)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        super().interpret(memory)


class InstructionDEFVAR(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        frame = self._get_frame_from_var_name()
        if not memory[frame]:
            error_exit(f"Frame {frame} is not initialized", 55)

        if memory[frame][0].get_variable(self.arg1.value[3:]) is not None:
            error_exit(f"Redefining variable {self.arg1.value}", 52)

        memory[frame][0].set_variable(self.arg1.value[3:])


class InstructionPUSHS(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        symb_value = self._get_value_from_symb(self.arg1, memory)

        memory["GF"][0].set_stack_var(symb_value)


class InstructionPOPS(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        value = memory["GF"][0].get_stack_var()
        variable = self._get_variable(self.arg1.value, memory)

        self.set_value(variable, value)


class DoubleArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        arg = self.raw_instruction.find("arg1")
        arg2 = self.raw_instruction.find("arg2")
        self.check_parameter_attributes(arg)
        self.check_parameter_attributes(arg2)

        self.check_argument(0, arg)
        self.check_argument(1, arg2)
        self.arg1 = self.get_argument(arg)
        self.arg2 = self.get_argument(arg2)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        super().interpret(memory)


class InstructionMOVE(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        variable = self._get_variable(self.arg1.value, memory)
        symb_value = self._get_value_from_symb(self.arg2, memory)

        self.set_value(variable, symb_value)


class InstructionNOT(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        result = self._get_variable(self.arg1.name, memory)
        if result.initialized and result.var_type != bool:
            error_exit(f"Invalid target variable type: {result.name}", 53)

        to_not = self._get_value_from_symb(self.arg2, memory)
        if not isinstance(to_not, bool):
            error_exit(f"Invalid operand {self.arg2.name} type!", 53)

        self.set_value(result, not to_not)


class TripleArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        arg = self.raw_instruction.find("arg1")
        arg2 = self.raw_instruction.find("arg2")
        arg3 = self.raw_instruction.find("arg3")
        self.check_parameter_attributes(arg)
        self.check_parameter_attributes(arg2)
        self.check_parameter_attributes(arg3)

        self.check_argument(0, arg)
        self.check_argument(1, arg2)
        self.check_argument(2, arg3)
        self.arg1 = self.get_argument(arg)
        self.arg2 = self.get_argument(arg2)
        self.arg3 = self.get_argument(arg3)


class MathInstruction(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def _get_value_from_symb(self, symb: Union[ConstantArgument, VariableArgument, ArgumentType], memory):
        if isinstance(symb, VariableArgument):
            variable = self._get_variable(symb.name, memory)
            if variable.initialized and variable.var_type != int:
                error_exit(f"Incompatible operand {symb.name} type!", 53)

            return variable.value
        else:
            constant = super()._get_value_from_symb(symb, memory)
            if not isinstance(constant, int):
                error_exit(f"Incompatible operand {symb.name} type!", 53)

            return constant

    def calculate(self, first: int, second: int) -> int:
        raise NotImplementedError("Needs to be implemented in inherited classes")

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        result = self._get_variable(self.arg1.name, memory)
        if result.initialized and result.var_type != int:
            error_exit(f"Incompatible result variable {result.name} type!", 53)

        first = self._get_value_from_symb(self.arg2, memory)
        second = self._get_value_from_symb(self.arg3, memory)

        self.set_value(result, self.calculate(first, second))


class InstructionADD(MathInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def calculate(self, first: int, second: int) -> int:
        return first + second


class InstructionSUB(MathInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def calculate(self, first: int, second: int) -> int:
        return first - second


class InstructionMUL(MathInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def calculate(self, first: int, second: int) -> int:
        return first * second


class InstructionIDIV(MathInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def calculate(self, first: int, second: int) -> int:
        if second == 0:
            error_exit("Division by zero!", 57)

        return first // second


class LogicalInstruction(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __get_result_variable(self, memory):
        result = self._get_variable(self.arg1.name, memory)
        if result.initialized and result.var_type != bool:
            error_exit(f"Invalid target variable type: {result.name}", 53)

        return result

    def get_values(self, memory):
        first = self._get_value_from_symb(self.arg2, memory)
        second = self._get_value_from_symb(self.arg3, memory)

        if not isinstance(first, type(second)) or isinstance(first, type(None)):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first, second

    def compare(self, first, second, memory):
        raise NotImplementedError("Should be evaluated in inherited classes")

    def interpret(self, memory: Dict[str, List[MemoryFrame]]):
        first, second = self.get_values(memory)
        result = self.__get_result_variable(memory)

        self.set_value(result, self.compare(first, second, memory))


class InstructionLT(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second, memory):
        return first < second


class InstructionGT(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second, memory):
        return first > second


class InstructionEQ(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def get_values(self, memory):
        first = self._get_value_from_symb(self.arg2, memory)
        second = self._get_value_from_symb(self.arg3, memory)

        if not isinstance(first, type(second)) and \
                not (isinstance(first, type(None)) or isinstance(second, type(None))):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first, second

    def compare(self, first, second, memory):
        return first == second


class InstructionAND(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second, memory):
        if not isinstance(first, bool) or not isinstance(second, bool):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first and second


class InstructionOR(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second, memory):
        if not isinstance(first, bool) or not isinstance(second, bool):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first or second


class Interpret:
    def __init__(self):
        self.arg_parser = argparse.ArgumentParser(description='IPPcode22 interpret')

        self.source_code = None
        self.inputs = None

        self._instructions: List[ET.ElementTree] = []
        self._parsed: List[Instruction] = []

        self.frames: Dict[str, List[MemoryFrame]] = {
            "GF": [MemoryFrame(True, None)],
            "TF": [],
            "LF": []
        }

    def __register_arguments(self):
        group = self.arg_parser.add_mutually_exclusive_group(required=True)
        group.add_argument(
            '--source',
            help="Program source code",
            type=argparse.FileType('r', encoding='UTF-8'),
            default=sys.stdin
        )
        group.add_argument(
            '--input',
            help="Input data",
            type=argparse.FileType('r', encoding='UTF-8'),
            default=sys.stdin
        )

    def process_arguments(self):
        self.__register_arguments()
        args = self.arg_parser.parse_args()

        self.source_code = args.source
        self.inputs = args.input

    def load_instructions(self):
        tree = ET.parse(self.source_code)
        raw_instructions = tree.findall("instruction")
        instruction_count = len(raw_instructions)

        self._instructions = [None for _ in range(instruction_count)]
        for instruction in raw_instructions:
            if "order" not in instruction.keys() or "opcode" not in instruction.keys():
                error_exit("Invalid instruction attributes", 32)

            order = 1
            try:
                order = int(instruction.get("order"))
            except ValueError:
                error_exit("Invalid instruction order", 32)

            if self._instructions[order - 1]:
                error_exit("Duplicity of instruction order", 32)

            self._instructions[order - 1] = instruction
        # no need to check for `None` in list, all elements are occupied

    @staticmethod
    def __get_instruction_class(instruction: ET.ElementTree):
        allowed_tags = ["arg1", "arg2", "arg3"]
        children = [child for child in instruction.iter()]
        instruction = children[0]
        children = children[1:]

        for parameter in children:
            if parameter.tag not in allowed_tags:
                error_exit("Invalid instruction argument", 31)

        params_count = len(children)
        name = instruction.attrib["opcode"]  # TODO: check for opcode attr

        if params_count == 0:
            return name, globals()[f"Instruction{name.upper()}"]
        elif params_count == 1:
            return name, globals()[f"Instruction{name.upper()}"]
        elif params_count == 2:
            return name, globals()[f"Instruction{name.upper()}"]
        elif params_count == 3:
            return name, globals()[f"Instruction{name.upper()}"]
        else:
            error_exit("Invalid arguments count", 31)

    def parse_instructions(self):
        for instruction in self._instructions:
            name, ins_class = self.__get_instruction_class(instruction)

            self._parsed.append(ins_class(name, instruction))

    def run(self):
        self.load_instructions()
        self.parse_instructions()


if __name__ == "__main__":
    interpret = Interpret()
    interpret.process_arguments()
    interpret.run()
