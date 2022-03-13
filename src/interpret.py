import argparse
import sys
from dataclasses import dataclass
from typing import Union, List
import xml.etree.ElementTree as ET


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
                print("ERROR1")  # TODO

            try:
                order = int(instruction.get("order"))
            except:
                print("ERROR2")
                return

            if self._instructions[order - 1]:
                print("ERROR3")

            self._instructions[order - 1] = instruction
        # no need to check for `None` in list, all elements are occupied

        print(self._instructions)

    def run(self):
        self.process_instructions()


if __name__ == "__main__":
    interpret = Interpret()
    interpret.process_arguments()
    interpret.run()
