<?PHP
    ini_set('display_errors', 'stderr');

    class Errors{
        const INVALID_HEAD = 21;
        const INVALID_CODE = 22;
        const OTHER_ERR = 23;

        private $codes = array(
            self::INVALID_HEAD => "Invalid program head!",
            self::INVALID_CODE => "Invalid operation code!",
            self::OTHER_ERR => "Other error!"
        );

        function error_exit($exit_code)
        {
            fwrite(STDERR, $this->codes[$exit_code] . "\n");
            exit($exit_code);
        }
    }

    class ArgumentParser{
        private $argc;
        private $argv;

        public function __construct($argc, $argv)
        {
            $this->argc = $argc;
            $this->argv = $argv;
        }

        function print_help($exit_code)
        {
            echo "USIDZ\n";
            exit($exit_code);
        }

        function parse()
        {
            if ($this->argc > 1 and $this->argv[1] != "--help")
            {
                echo "Invalid usage!";
                $this->print_help(1);
            }else if ($this->argc > 1 and $this->argv[1] == "--help")
                $this->print_help(0);
        }
    }

    class InstructionParser{
        const __var = "[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*";
        const __const = "(?:int@(?:0[xX][0-9a-fA-F]+|[+-]?[0-9]+))|bool@(?:true|false)|nil@nil|string@(?:[^#\\\\\s]|\\\\\d{3})*";
        const insParamsRegexp = array(
            "var" => "\s+([LTG]F@" . self::__var . ")",
            "label" => "\s+(" . self::__var . ")",
            "symb" => "\s+((?:[LTG]F@" . self::__var . ")|(?:" . self::__const . "))",
            "type" => "\s+(int|string|bool)"
        );

        const instructions = array(
            0 => array(array("(CREATEFRAME)"), array("(PUSHFRAME)"), array("(POPFRAME)"), array("(RETURN)"), array("(BREAK)")),
            1 => array(
                array("(DEFVAR)", self::insParamsRegexp["var"]),
                array("(CALL)", self::insParamsRegexp["label"]),
                array("(PUSHS)", self::insParamsRegexp["symb"]),
                array("(POPS)", self::insParamsRegexp["var"]),
                array("(WRITE)", self::insParamsRegexp["symb"]),
                array("(LABEL)", self::insParamsRegexp["label"]),
                array("(JUMP)", self::insParamsRegexp["label"]),
                array("(EXIT)", self::insParamsRegexp["symb"]),
                array("(DPRINT)", self::insParamsRegexp["symb"])),
            2 => array(
                array("(MOVE)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"]),
                array("(INT2CHAR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"]),
                array("(READ)", self::insParamsRegexp["var"], self::insParamsRegexp["type"]),
                array("(STRLEN)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"]),
                array("(TYPE)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"])),
            3 => array(
                array("(ADD)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(SUB)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(MUL)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(IDIV)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(LT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(GT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(EQ)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(AND)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(OR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(NOT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(STRI2INT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(CONCAT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(GETCHAR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(SETCHAR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(JUMPIFEQ)", self::insParamsRegexp["label"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(JUMPIFNEQ)", self::insParamsRegexp["label"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]))
            );

        public function get_instruction($raw)
        {
            foreach (self::instructions as $instructionArrayIndex => $instructionArray)
            {
                foreach ($instructionArray as $instruction)
                {
                    $instruction = implode($instruction);

                    $regexps = explode("\s+", $instruction, 3);
                    if (substr($regexps[0], 1, -1) == strtoupper(explode(" ", $raw)[0]))
                    {
                        if (!preg_match("/^" . $instruction . "$/u", $raw, $regexps))
                            exit(23);

                        switch ($instructionArrayIndex)
                        {
                            case 0:
                                return new NoArgsInstruction($raw, $regexps);
                            case 1:
                                return new SingleArgsInstruction($raw, $regexps);
                            case 2:
                                return new DoubleArgsInstruction($raw, $regexps);
                            case 3:
                                return new TripleArgsInstruction($raw, $regexps);
                        }
                    }
                }
            }
            exit(22);
        }
    }

    class Instruction {
        private $raw;
        public $instruction;
        public $splitted = null;

        public function __construct($raw, $splitted)
        {
            $this->raw = $raw;
            $this->instruction = strtoupper($splitted[1]);
            $this->splitted = $splitted;
        }

        public function get_instruction($key, $xml_dom, $header)
        {
            $instruction = $xml_dom->createElement("instruction");
            $instruction->setAttribute("order", $key + 1);
            $instruction->setAttribute("opcode", $this->instruction);

            $instruction = $header->appendChild($instruction);
            $this->get_arguments($xml_dom, $instruction);
        }

        private function get_instruction_array_index()
        {
            if ($this instanceof NoArgsInstruction)
                throw new Exception("Instructions without arguments could not have any");
            elseif ($this instanceof SingleArgsInstruction)
                return 1;
            elseif ($this instanceof DoubleArgsInstruction)
                return 2;
            else
                return 3;
        }

        private function get_symb_attr_type($attr_index)
        {
            preg_match("/^(([LTG]F)|int|bool|nil|string)@.*/u", $this->splitted[$attr_index], $matches);
            if (count($matches) > 2 and $matches[2])
                return "var";

            return $matches[1];
        }

        private function get_attribute_type($param_index)
        {
            $instructions = InstructionParser::instructions[$this->get_instruction_array_index()];
            $instruction_array = null;

            foreach ($instructions as $instruction_array)
                if (substr($instruction_array[0], 1, -1) == $this->instruction)
                    break;

            $paramRegexp = $instruction_array[$param_index];

            switch ($paramRegexp)
            {
                case InstructionParser::insParamsRegexp["var"]:
                    return "var";
                case InstructionParser::insParamsRegexp["label"]:
                    return "label";
                case InstructionParser::insParamsRegexp["type"]:
                    return "type";
                case InstructionParser::insParamsRegexp["symb"]:
                    return $this->get_symb_attr_type($param_index + 1);
                default:
                    throw new Exception("Another operand type is not implemented");
            }
        }

        private function get_value_from_symb($symb)
        {
            return substr($symb, strpos($symb, "@") + 1);
        }

        public function generate_argument($xml_dom, $node, $attr_index)
        {
            $type = $this->get_attribute_type($attr_index);
            $arguments = $xml_dom->createElement("arg" . $attr_index);
            $arguments->setAttribute("type", $type);

            $value = $this->splitted[$attr_index + 1];
            if (in_array($type, array("int", "bool", "nil", "string")))
                $value = $this->get_value_from_symb($value);

            $arguments->textContent = $value;  // arguments start at 2nd position
            $node->appendChild($arguments);
        }

        public function get_arguments($xml_dom, $node)
        {
            throw new Exception("Not implemented");
        }

        public function __toString()
        {
            return $this->raw;
        }
    }

    class NoArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
        }
    }

    class SingleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);
        }
    }

    class DoubleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);
            $this->generate_argument($xml_dom, $node, 2);
        }
    }

    class TripleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);
            $this->generate_argument($xml_dom, $node, 2);
            $this->generate_argument($xml_dom, $node, 3);
        }
    }

    class Parser{
        private $line_number = -1;
        private $errors;

        public function __construct()
        {
            $this->errors = new Errors();
        }

        function remove_comments($line)
        {
            $pos = strrpos($line, "#");

            if($pos !== false)
                return substr($line, 0, strpos($line, "#"));
            return $line;
        }

        function remove_newline($line)
        {
            $pos = strrpos($line, "\n");

            if($pos !== false)
                $line = substr_replace($line, "", $pos, 1);
            return $line;
        }

        function remove_whitespaces($line)
        {
            return trim($line);
        }

        function prepare_line($line)
        {
            $line = $this->remove_newline($line);
            $line = $this->remove_comments($line);
            return $this->remove_whitespaces($line);
        }

        function parse_lines()
        {
            $got_header = false;
            $instruction_parser = new InstructionParser();
            $instructions = new SplDoublyLinkedList();

            while($line = fgets(STDIN))
            {
                $line = $this->prepare_line($line);
                $this->line_number++;

                if (strlen($line) == 0)
                    continue;

                if (!$got_header and $line != ".IPPcode22")
                    $this->errors->error_exit(21);
                elseif(!$got_header and $line == ".IPPcode22")
                {
                    $got_header = true;
                    continue;
                }

                $parsed_instruction = $instruction_parser->get_instruction($line);
                $instructions->add($this->line_number - 1, $parsed_instruction);
            }
            return $instructions;
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $parser = new Parser();
    $instructions = $parser->parse_lines();

    $xml = new DOMDocument('1.0', 'UTF-8');
    $xml->formatOutput = true;
    $header = $xml->createElement("program");
    $header->setAttribute("language", "IPPcode22");
    $xml->appendChild($header);


    $instructions->setIteratorMode(SplDoublyLinkedList::IT_MODE_FIFO);

    for ($instructions->rewind(); $instructions->valid(); $instructions->next())
        $instructions->current()->get_instruction($instructions->key(), $xml, $header);

    echo $xml->saveXML();


