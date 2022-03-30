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
                self.initialized = True

            super().__setattr__(key, value)

        def __getattr__(self, item):
            if item == "value" and not self.initialized:
                error_exit("Variable is not initialized", 56)

            return super().__getattribute__(item)

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
        self.type = self.set_type()

    def set_type(self):
        if self.value == "int":
            return int
        elif self.value == "string":
            return str
        elif self.value == "bool":
            return bool

        error_exit("Invalid type!", 53)

    def set_value(self):
        self.value = self.raw_element.text


class Instruction:
    __VAR_NAME_REGEXP = r"[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*"
    __CONSTANTS_REGEXP = r"(?:(?:0[xX][0-9a-fA-F]+|[+-]?[0-9]+))|(?:true|false)|nil|(?:[^#\\\\\s]|\\\d{3})*"
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

    def __init__(self, name: str, xml_raw: ET.ElementTree, memory: Dict[str, List[MemoryFrame]]):
        self.name = name.upper()
        self.raw_instruction = xml_raw
        self.ins_regexp = self.get_instruction_regexp()
        self._prev: Instruction = None
        self._next: Instruction = None

        self.arg1: ArgumentType = None
        self.arg2: ArgumentType = None
        self.arg3: ArgumentType = None
        self.set_arguments()

        self.memory = memory

    def __repr__(self):
        ret = f"{self.__class__.__name__} - {self.name} \n"
        ret += f"\t{self.arg1}\n"
        ret += f"\t{self.arg2}\n"
        ret += f"\t{self.arg3}"
        return ret

    @property
    def next(self):
        return self._next

    @next.setter
    def next(self, value: "Instruction"):
        self._next = value

    @property
    def prev(self):
        return self._prev

    @prev.setter
    def prev(self, value: "Instruction"):
        self._prev = value

    @staticmethod
    def check_parameter_attributes(parameter: ET.Element):
        if parameter is not None and "type" in parameter.keys() and len(parameter.items()) == 1:
            return True
        error_exit("Invalid param attributes", 31)

    """
        @brief JUMPIFEQ is inherited from SingleArgsIns and also TripleArgsIns,
        so it needs to find parent with highest # of args to parse them all correctly
    """
    def __get_instruction_array_index(self):
        ret = -1

        if isinstance(self, NoArgsInstruction):
            ret = 0
        if isinstance(self, SingleArgsInstruction):
            ret = 1
        if isinstance(self, DoubleArgsInstruction):
            ret = 2
        if isinstance(self, TripleArgsInstruction):
            ret = 3

        if ret == -1:
            raise NotImplementedError("Instruction with >3 arguments is not (yet) supported")
        return ret

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

    def _get_variable(self, name: str) -> "MemoryFrame.Variable":
        frame = self._get_frame_from_var_name(name)
        if not self.memory[frame]:
            error_exit(f"Frame {frame} is not initialized", 55)

        variable = self.memory[frame][0].get_variable(name[3:])
        if variable is None:
            error_exit(f"Undefined variable {name}", 54)

        return variable

    def __encode_string(self, value: str):
        match = re.search(r"\\(\d{3})", value)
        if match:
            value = value[:match.start()] + chr(int(match.group(1))) + self.__encode_string(value[match.end():])

        return value

    def __get_value_from_constant(self, const: ConstantArgument):
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
            return self.__encode_string(const.value)
        elif const.type == type(None):
            return None

        error_exit("Unsupported constant value", 52)

    def _get_value_from_symb(self, symb: Union[ConstantArgument, VariableArgument, ArgumentType]):
        if isinstance(symb, ConstantArgument):
            return self.__get_value_from_constant(symb)
        else:
            variable = self._get_variable(symb.name)
            return variable.value

    def interpret(self):
        raise NotImplementedError()


class NoArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        pass

    def interpret(self):
        super().interpret()


class InstructionCREATEFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        self.memory["TF"] = [MemoryFrame(True, None)]


class InstructionPUSHFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        if not self.memory["TF"]:
            error_exit("No frame to push!", 55)

        self.memory["LF"].append(self.memory["TF"][0])
        self.memory["TF"] = []


class InstructionPOPFRAME(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        if not self.memory["LF"]:
            error_exit("No frame to pop!", 55)

        self.memory["TF"] = [self.memory["LF"].pop()]


class InstructionBREAK(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        print(self.memory, file=sys.stderr)


class InstructionRETURN(NoArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._jump_dest = None

    @property
    def next(self):
        if self._jump_dest:
            dest = self._jump_dest
            self._jump_dest = None

            return dest

        return self._next

    @next.setter
    def next(self, value):
        self._next = value

    def interpret(self):
        try:
            self._jump_dest = self.memory["--CALL_STACK"].pop()
        except IndexError:
            error_exit("Nowhere to return!", 56)


class SingleArgsInstruction(Instruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, *kwargs)

    def set_arguments(self):
        arg = self.raw_instruction.find("arg1")
        self.check_parameter_attributes(arg)

        self.check_argument(0, arg)
        self.arg1 = self.get_argument(arg)

    def interpret(self):
        super().interpret()


class InstructionDEFVAR(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        frame = self._get_frame_from_var_name()
        if not self.memory[frame]:
            error_exit(f"Frame {frame} is not initialized", 55)

        if self.memory[frame][0].get_variable(self.arg1.value[3:]) is not None:
            error_exit(f"Redefining variable {self.arg1.value}", 52)

        self.memory[frame][0].set_variable(self.arg1.value[3:])


class InstructionPUSHS(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        symb_value = self._get_value_from_symb(self.arg1)

        self.memory["GF"][0].set_stack_var(symb_value)


class InstructionPOPS(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        value = self.memory["GF"][0].get_stack_var()
        variable = self._get_variable(self.arg1.value)

        variable.value = value


class InstructionWRITE(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        to_print = self._get_value_from_symb(self.arg1)

        if to_print is None:
            to_print = ""
        elif to_print == True:
            to_print = "true"
        elif to_print == False:
            to_print = "false"

        print(to_print, end="")


class InstructionDPRINT(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        to_print = self._get_value_from_symb(self.arg1)

        print(to_print, file=sys.stderr, end="")


class InstructionEXIT(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        code = self._get_value_from_symb(self.arg1)

        if not isinstance(code, int) or code < 0 or code > 49:
            error_exit("Invalid exit code", 57)

        raise SystemExit(code)


class InstructionLABEL(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.label_name = self.arg1.value

    def _check_duplicity(self):
        prev = self.prev

        while prev:
            if isinstance(prev, InstructionLABEL) and prev.label_name == self.label_name:
                raise ValueError(f"Redefining label: {self.label_name}")
            prev = prev.prev

    def interpret(self):
        try:
            self._check_duplicity()
        except ValueError as e:
            error_exit(str(e), 52)


class InstructionJUMP(SingleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._jump_dest = None

    @property
    def next(self):
        if self._jump_dest:
            dest = self._jump_dest
            self._jump_dest = None

            return dest

        return self._next

    @next.setter
    def next(self, value):
        self._next = value

    def _get_jump_destination(self):
        prev = self.prev
        while prev:
            if isinstance(prev, InstructionLABEL) and prev.label_name == self.arg1.value:
                return prev
            prev = prev.prev

        next_ins = self.next
        while next_ins:
            if isinstance(next_ins, InstructionLABEL) and next_ins.label_name == self.arg1.value:
                return next_ins
            next_ins = next_ins.next

        return None

    def _check_jump_dest(self):
        destination = self._get_jump_destination()
        if not destination:
            error_exit("Could not locate jump", 52)

        return destination

    def interpret(self):
        destination = self._check_jump_dest()

        self._jump_dest = destination


class InstructionCALL(InstructionJUMP):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        self.memory["--CALL_STACK"].append(self.next)

        super().interpret()


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

    def interpret(self):
        super().interpret()


class InstructionMOVE(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        variable = self._get_variable(self.arg1.value)
        symb_value = self._get_value_from_symb(self.arg2)

        variable.value = symb_value


class InstructionNOT(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        to_not = self._get_value_from_symb(self.arg2)
        if not isinstance(to_not, bool):
            error_exit(f"Invalid operand {self.arg2.name} type!", 53)

        result.value = (not to_not)


class InstructionINT2CHAR(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        convert = self._get_value_from_symb(self.arg2)
        if not isinstance(convert, int):
            error_exit(f"Invalid operand {self.arg2.name} type!", 53)

        try:
            converted = chr(convert)
        except ValueError:
            error_exit("Invalid ordinal number", 58)
            return

        result.value = converted


class InstructionTYPE(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @staticmethod
    def __get_type(value):
        if isinstance(value, int) or value == int:
            return "int"
        elif isinstance(value, str) or value == str:
            return "string"
        elif isinstance(value, bool) or value == bool:
            return "bool"
        elif isinstance(value, type(None)) or value is None:
            return "nil"

        error_exit("Invalid constant type", 53)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        if isinstance(self.arg2, ConstantArgument):
            var_type = self.__get_type(self.arg2.type)
        else:
            variable = self._get_variable(self.arg2.name)
            if variable.initialized:
                var_type = self.__get_type(variable.value)
            else:
                var_type = ""

        result.value = var_type


class InstructionSTRLEN(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)
        operand = self._get_value_from_symb(self.arg2)
        if not isinstance(operand, str):
            error_exit(f"Invalid operand {self.arg2.name} type!", 53)

        result.value = len(operand)


class InstructionREAD(DoubleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __get_type(self):
        if self.arg2.value == "int":
            return int
        elif self.arg2.value == "string":
            return str
        elif self.arg2.value == "bool":
            return bool
        else:
            raise ValueError("This should never happen, check regexp")

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        var_type = self.__get_type()
        value = None

        try:
            raw_value = input()
            value = (var_type)(raw_value)  # C style conversion
        except (ValueError, TypeError):  # invalid type conversion
            pass
        except EOFError:  # invalid file
            pass

        result.value = value  # TODO: maybe set None also when file does not exists


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

    def _get_value_from_symb(self, symb: Union[ConstantArgument, VariableArgument, ArgumentType]):
        if isinstance(symb, VariableArgument):
            variable = self._get_variable(symb.name)
            if variable.initialized and variable.var_type != int:
                error_exit(f"Incompatible operand {symb.name} type!", 53)

            return variable.value
        else:
            constant = super()._get_value_from_symb(symb)
            if not isinstance(constant, int):
                error_exit(f"Incompatible operand {symb.name} type!", 53)

            return constant

    def calculate(self, first: int, second: int) -> int:
        raise NotImplementedError("Needs to be implemented in inherited classes")

    def interpret(self):
        result = self._get_variable(self.arg1.name)
        if result.initialized and result.var_type != int:
            error_exit(f"Incompatible result variable {result.name} type!", 53)

        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        result.value = self.calculate(first, second)


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

    def get_values(self):
        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        if not isinstance(first, type(second)) or isinstance(first, type(None)):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first, second

    def compare(self, first, second):
        raise NotImplementedError("Should be evaluated in inherited classes")

    def interpret(self):
        first, second = self.get_values()
        result = self._get_variable(self.arg1.name)

        result.value = self.compare(first, second)


class InstructionLT(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second):
        return first < second


class InstructionGT(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second):
        return first > second


class InstructionEQ(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def get_values(self):
        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        if not isinstance(first, type(second)) and \
                not (isinstance(first, type(None)) or isinstance(second, type(None))):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first, second

    def compare(self, first, second):
        return first == second


class InstructionAND(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second):
        if not isinstance(first, bool) or not isinstance(second, bool):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first and second


class InstructionOR(LogicalInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def compare(self, first, second):
        if not isinstance(first, bool) or not isinstance(second, bool):
            error_exit(f"Invalid operand types: {self.arg2.name}, {self.arg3.name}", 53)

        return first or second


class InstructionSTRI2INT(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        if not isinstance(first, str) or not isinstance(second, int):
            error_exit(f"Invalid operand {self.arg2.name} or {self.arg3.name} types!", 53)

        if second >= len(first):
            error_exit(f"Index {self.arg3.name} out of range in {self.arg2.name}", 58)

        result.value = ord(first[second])


class InstructionCONCAT(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        if not isinstance(first, str) or not isinstance(second, str):
            error_exit(f"Invalid operand {self.arg2.name} or {self.arg3.name} types!", 53)

        result.value = first + second


class InstructionGETCHAR(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)

        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        if not isinstance(first, str) or not isinstance(second, int):
            error_exit(f"Invalid operand {self.arg2.name} or {self.arg3.name} types!", 53)

        if second >= len(first):
            error_exit(f"Character out of bonds: {self.arg3.name}", 58)

        result.value = first[second]


class InstructionSETCHAR(TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def interpret(self):
        result = self._get_variable(self.arg1.name)
        to_replace = list(self._get_value_from_symb(self.arg1))

        replace_with = self._get_value_from_symb(self.arg3)[0]
        replace_at = self._get_value_from_symb(self.arg2)

        to_replace[replace_at] = replace_with[0]
        result.value = "".join(to_replace)


class InstructionJUMPIFEQ(InstructionJUMP, TripleArgsInstruction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        for base in InstructionJUMPIFEQ.__bases__:
            if base == TripleArgsInstruction:
                base.set_arguments(self)
                break

    def _compare(self, first, second):
        return first == second

    def interpret(self):
        first = self._get_value_from_symb(self.arg2)
        second = self._get_value_from_symb(self.arg3)

        first_type = type(first)
        second_type = type(second)

        if (first_type == second_type or first_type == type(None) or second_type == type(None)) and \
                self._check_jump_dest() and self._compare(first, second):
            super().interpret()


class InstructionJUMPIFNEQ(InstructionJUMPIFEQ):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def _compare(self, first, second):
        return first != second


class Interpret:
    def __init__(self):
        self.arg_parser = argparse.ArgumentParser(description='IPPcode22 interpret')

        self.source_code = None

        self._instructions: List[ET.ElementTree] = []

        self.frames: Dict[str, List[MemoryFrame]] = {
            "GF": [MemoryFrame(True, None)],
            "TF": [],
            "LF": [],
            "--CALL_STACK": []
        }
        self.IP: List[Instruction] = []  # next instruction pointers stack

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

        if args.input != sys.stdin:
            sys.stdin = args.input

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
        prev = None

        for instruction in self._instructions:
            name, ins_class = self.__get_instruction_class(instruction)
            initialized: Instruction = ins_class(name, instruction, self.frames)

            initialized.prev = prev
            if prev:
                prev.next = initialized
            prev = initialized

        return prev  # it returns the last instruction

    def prepare_instruction_ptr(self, last: Instruction):
        while last.prev:
            last = last.prev

        self.IP.append(last)

    def run(self):
        self.load_instructions()
        last_ins = self.parse_instructions()
        self.prepare_instruction_ptr(last_ins)

        while self.IP:
            to_run = self.IP.pop()
            try:
                to_run.interpret()
            except SystemExit as e:  # used for `EXIT` instruction
                exit(e.code)

            if to_run._next:
                self.IP.append(to_run.next)


if __name__ == "__main__":
    interpret = Interpret()
    interpret.process_arguments()
    interpret.run()
