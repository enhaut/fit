import argparse
import sys
from dataclasses import dataclass
from typing import Union, List
import xml.etree.ElementTree as ET


def error_exit(message: str, code: int):
    print(message, file=sys.stderr)
    sys.exit(code)


class MemoryFrame:
    @dataclass
    class Variable:
        name: str
        value: Union[str, int, float, bool, None]
        initialized: bool = False

    def __init__(self, initialized=False, parent="MemoryFrame"):
        self._initialized = initialized
        self._variables: List["MemoryFrame.Variable"] = []
        self._parent = parent


class Instruction:
    __VAR_NAME_REGEXP = "[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*"
    __CONSTANTS_REGEXP= "(?:int@(?:0[xX][0-9a-fA-F]+|[+-]?[0-9]+))|bool@(?:true|false)|nil@nil|string@(?:[^#\\\\\s]|\\\\\d{3})*"
    VAR_REGEXPS = {
        "var": "\s+([LTG]F@" + __VAR_NAME_REGEXP + ")",
        "label": "\s+("  + __VAR_NAME_REGEXP + ")",
        "symb": "\s+((?:[LTG]F@"  + __VAR_NAME_REGEXP + ")|(?:"  + __CONSTANTS_REGEXP + "))",
        "type": "\s+(int|string|bool)"
    }
    INSTRUCTIONS = [
        [("(CREATEFRAME)",), ("(PUSHFRAME)",), ("(POPFRAME)",), ("(RETURN)",), ("(BREAK)",)],
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
            ("NOT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("STRI2INT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("CONCAT", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("GETCHAR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("SETCHAR", VAR_REGEXPS["var"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("JUMPIFEQ", VAR_REGEXPS["label"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"]),
            ("JUMPIFNEQ", VAR_REGEXPS["label"], VAR_REGEXPS["symb"], VAR_REGEXPS["symb"])
        ]
    ]
    def __init__(self, name: str, xml_raw: ET.ElementTree):
        pass


class Interpret:
    def __init__(self):
        self.arg_parser = argparse.ArgumentParser(description='IPPcode22 interpret')

        self.source_code = None
        self.inputs = None

        self._instructions: List = []

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

    def process_instructions(self):
        tree = ET.parse(self.source_code)
        raw_instructions = tree.findall("instruction")
        instruction_count = len(raw_instructions)

        self._instructions = [None for _ in range(instruction_count)]
        for instruction in raw_instructions:
            if "order" not in instruction.keys() or "opcode" not in instruction.keys():
                error_exit("Invalid instruction attributes", 32)

            try:
                order = int(instruction.get("order"))
            except:
                error_exit("Invalid instruction order", 32)
                return

            if self._instructions[order - 1]:
                error_exit("Duplicity of instruction order", 32)

            self._instructions[order - 1] = instruction
        # no need to check for `None` in list, all elements are occupied

        print(self._instructions)

    def run(self):
        self.process_instructions()


if __name__ == "__main__":
    interpret = Interpret()
    interpret.process_arguments()
    interpret.run()
