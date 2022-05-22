<?PHP
    ini_set('display_errors', 'stderr');

/**
 * @brief Class is responsible for handling errors.
 * Keep all the error codes here as a constant.
 */
    class Errors{
        const INVALID_HEAD = 21;
        const INVALID_CODE = 22;
        const OTHER_ERR = 23;

        private $codes = array(
            self::INVALID_HEAD => "Invalid program head!",
            self::INVALID_CODE => "Invalid operation code!",
            self::OTHER_ERR => "Other error!"
        );

        /**
         * @brief Method should be called to exit running
         * script with error code. Error message from
         * `Errors::$codes` is then printed to `stderr`.
         *
         * @param $exit_code int script exit code
         * @return void
         */
        function error_exit($exit_code)
        {
            fwrite(STDERR, $this->codes[$exit_code] . "\n");
            exit($exit_code);
        }
    }

/**
 * @brief Class is responsible for handling
 * program arguments.
 */
    class ArgumentParser{
        private $argc;
        private $argv;

        /**
         * @param $argc int number of total arguments
         * @param $argv array array of arguments
         */
        public function __construct($argc, $argv)
        {
            $this->argc = $argc;
            $this->argv = $argv;
        }

        /**
         * @brief Function prints usage information
         * and exits with `$exit_code`.
         *
         * @param $exit_code int program return code
         * @return void
         */
        function print_help($exit_code)
        {
            echo "Usage: php parse.php < program.src";
            exit($exit_code);
        }

        /**
         * @brief Method parses program arguments.
         *
         * @return void
         */
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

/**
 * @brief Class responsible for parsing instructions.
 */
    class InstructionParser{
        const __var = "[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*";
        /* ^variable could start with alphanumeric characters + special characters,
        *  the rest of name could also contain numbers
        */
        const __const = "(?:int@(?:0(?:[xX][0-9a-fA-F]+|[oO][0-7]+)|[+-]?[0-9]+))|bool@(?:true|false)|nil@nil|string@(?:[^#\\\\\s]|\\\\\d{3})*";
        const insParamsRegexp = array(
            "var" => "\s+([LTG]F@" . self::__var . ")",
            "label" => "\s+(" . self::__var . ")",
            "symb" => "\s+((?:[LTG]F@" . self::__var . ")|(?:" . self::__const . "))",
            "type" => "\s+(int|string|bool)"
        );

        /**
         * Instructions are ordered to array (based on # of parameters) of arrays (instruction name, parameters regexp)
         */
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
                array("(NOT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"]),
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
                array("(STRI2INT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(CONCAT)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(GETCHAR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(SETCHAR)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(JUMPIFEQ)", self::insParamsRegexp["label"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(JUMPIFNEQ)", self::insParamsRegexp["label"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]))
            );

        /**
         * @brief Method returns corresponding initialized instruction class.
         *
         * @param $raw string raw instruction string
         * @return DoubleArgsInstruction|NoArgsInstruction|SingleArgsInstruction|TripleArgsInstruction
         */
        public function get_instruction($raw)
        {
            foreach (self::instructions as $instructionArrayIndex => $instructionArray)  // cycle over instruction params #
            {
                foreach ($instructionArray as $instruction)  // cycle over instructions
                {
                    $instruction = implode($instruction);

                    $regexps = explode("\s+", $instruction, 3);  // split instruction and parameters
                    $exploded = explode(" ", $raw);
                    $upper = strtoupper($exploded[0]);  // instructions are case-insensitive

                    if (substr($regexps[0], 1, -1) == $upper)
                    {
                        $raw = substr_replace($raw, $upper, strpos($raw, $exploded[0]), strlen($exploded[0]));
                        if (!preg_match("/^" . $instruction . "$/u", $raw, $regexps))
                            exit(23);  // invalid parameters

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
            exit(22);  // invalid instruction
        }
    }

/**
 * @brief Base parent class for all the instructions.
 * This class should NOT be used by itself. Use its
 * children instead.
 */
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

        /**
         * @brief Method prepares and adds instruction to
         * be added to output XML.
         *
         * @param $key int instruction order
         * @param $xml_dom DOMDocument reference to base XMLDOM
         * @param $header DOMElement <program> element, it should contains
         * all the instructions
         * @return void
         */
        public function get_instruction($key, $xml_dom, $header)
        {
            $instruction = $xml_dom->createElement("instruction");
            $instruction->setAttribute("order", $key + 1);
            $instruction->setAttribute("opcode", $this->instruction);

            $instruction = $header->appendChild($instruction);
            $this->get_arguments($xml_dom, $instruction);
        }

        /**
         * @brief Method returns index of instruction
         * position in `InstructionsParser::instructions` array.
         *
         * @return int instruction array index
         */
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

        /**
         * @brief Method returns type of used <symbol>
         * as a parameter in instruction call.
         *
         * @param $attr_index int index of attribute in parsed instruction array
         * @return string
         */
        private function get_symb_attr_type($attr_index)
        {
            preg_match("/^(([LTG]F)|int|bool|nil|string)@.*/u", $this->splitted[$attr_index], $matches);
            if (count($matches) > 2 and $matches[2])
                return "var";

            return $matches[1];
        }

        /**
         * @brief Method returns type of attribute
         * used in instruction call.
         * Method looks up for instruction in `InstructionParser::instruction`
         * array and tries to match `$params_index`'s value by instruction
         * param regexp.
         *
         * @param $param_index int index of parameter in parsed instruction array
         * @return string type of parameter
         */
        private function get_attribute_type($param_index)
        {
            $instructions = InstructionParser::instructions[$this->get_instruction_array_index()];
            $instruction_array = null;

            foreach ($instructions as $instruction_array)  // looking up for allowed instructions parameters
                if (substr($instruction_array[0], 1, -1) == $this->instruction)
                    break;

            $paramRegexp = $instruction_array[$param_index];  // get corresponding regexp

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

        /**
         * @brief Method returns value of symbol.
         * For example symbol `int@5` has value of `5`.
         *
         * @param $symb string raw <symbol>
         * @return string symbol value
         */
        private function get_value_from_symb($symb)
        {
            return substr($symb, strpos($symb, "@") + 1);
        }

        /**
         * @brief Method prepares and adds instruction's call
         * parameters to the XML.
         *
         * @param $xml_dom DOMDocument base object of XML
         * @param $node DOMElement <instruction> element (parent to parameters)
         * @param $attr_index int added attribute index
         * @return void
         */
        public function generate_argument($xml_dom, $node, $attr_index)
        {
            $type = $this->get_attribute_type($attr_index);
            $arguments = $xml_dom->createElement("arg" . $attr_index);  // generate <arg{$attr_index}> elem
            $arguments->setAttribute("type", $type);

            $value = $this->splitted[$attr_index + 1];
            if (in_array($type, array("int", "bool", "nil", "string")))  // type is <symbol>
                $value = $this->get_value_from_symb($value);

            $arguments->textContent = $value;
            $node->appendChild($arguments);
        }

        /**
         * @brief Method should be overloaded
         * in children classes. It should implement
         * getting arguments for instruction call.
         *
         * @param $xml_dom DOMDocument the base XML document
         * @param $node DOMElement <program> element
         */
        public function get_arguments($xml_dom, $node)
        {
            throw new Exception("Not implemented");
        }

        public function __toString()
        {
            return $this->raw;
        }
    }

/**
 * @brief Class handles instruction used
 * **WITHOUT** any parameters.
 */
    class NoArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
        }
    }

/**
 * @brief Class implements parsing
 * instruction that uses 1 parameter.
 */
    class SingleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);  // <arg1>
        }
    }

/**
 * @brief Class implements parsing
 * instructions that use 2 parameters.
 */
    class DoubleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);  // <arg1>
            $this->generate_argument($xml_dom, $node, 2);  // <arg2>
        }
    }

/**
 * @brief Class implements parsing
 * instructions that use 3 parameters.
 */
    class TripleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            $this->generate_argument($xml_dom, $node, 1);  // <arg1>
            $this->generate_argument($xml_dom, $node, 2);  // <arg2>
            $this->generate_argument($xml_dom, $node, 3);  // <arg3>
        }
    }

/**
 * @brief Class parses input from `stdin`
 * and handles all the regarding stuff.
 */
    class Parser{
        private $line_number = -1;
        private $list_index = 0;  // in case of empty lines, DLL has counter just for itself
        private $errors;

        public function __construct()
        {
            $this->errors = new Errors();
        }

        /**
         * @brief Method removes comments from raw line.
         *
         * @param $line string raw line
         * @return string
         */
        function remove_comments($line)
        {
            $pos = strrpos($line, "#");

            if($pos !== false)
                return substr($line, 0, strpos($line, "#"));
            return $line;
        }

        /**
         * @brief Method removed new line character
         * from the raw line.
         *
         * @param $line string raw line
         * @return string
         */
        function remove_newline($line)
        {
            $pos = strrpos($line, "\n");

            if($pos !== false)
                $line = substr_replace($line, "", $pos, 1);
            return $line;
        }

        /**
         * @brief Method removes white spaces around raw.
         *
         * @param $line string raw line
         * @return string
         */
        function remove_whitespaces($line)
        {
            return trim($line);
        }

        /**
         * @brief Method removes all unnecessary
         * characters from raw instruction.
         *
         * @param $line string raw line
         * @return string
         */
        function prepare_line($line)
        {
            $line = $this->remove_newline($line);
            $line = $this->remove_comments($line);
            return $this->remove_whitespaces($line);
        }

        /**
         * @brief Method parses `stdin` input and
         * returns `SplDoublyLinkedList` with
         * parsed instruction classes. DLL could be then
         * used to optimize code. But code optimization
         * is not (yet) implemented.
         *
         * @return SplDoublyLinkedList
         */
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
                    continue;  // skip empty lines

                if (!$got_header and $line != ".IPPcode22")
                    $this->errors->error_exit(21);  // program header is mandatory
                elseif(!$got_header and $line == ".IPPcode22")
                {
                    $got_header = true;
                    continue;
                }

                $parsed_instruction = $instruction_parser->get_instruction($line);
                $instructions->add($this->list_index, $parsed_instruction);
                $this->list_index++;
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
    $header = $xml->createElement("program");  // <instruction>s should be children of <program> element
    $header->setAttribute("language", "IPPcode22");
    $xml->appendChild($header);


    $instructions->setIteratorMode(SplDoublyLinkedList::IT_MODE_FIFO);

    for ($instructions->rewind(); $instructions->valid(); $instructions->next())
        $instructions->current()->get_instruction($instructions->key(), $xml, $header);  // add instructions to XML

    echo $xml->saveXML();
