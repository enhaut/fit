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
        private int $argc;
        private array $argv;

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
        private const __var = "[a-zA-Z_\-$&%*!?][a-zA-Z0-9_\-$&%*!?]*";
        private const __const = "(int@((0[xX][0-9a-fA-F]+)|([+-]?[0-9]+)))|string@([^#\\\s]|\\\d{3})*|bool@(true|false)|(nil@nil)";
        private const insParamsRegexp = array(
            "var" => "\s+([LTG]F@" . self::__var . ")",
            "label" => "\s+(" . self::__var . ")",
            "symb" => "\s+(([LTG]F@" . self::__var . ")|(" . self::__const . "))",
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
                array("(JUMPIFEQ)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]),
                array("(JUMPIFNEQ)", self::insParamsRegexp["var"], self::insParamsRegexp["symb"], self::insParamsRegexp["symb"]))
            );

        public function get_instruction($raw)
        {
            foreach (self::instructions as $instructionArrayIndex => $instructionArray)
            {
                foreach ($instructionArray as $instruction)
                {
                    $instruction = implode($instruction);

                    $regexps = explode("\s+", $instruction, 3);
                    if (preg_match("/". $regexps[0] . "/u", $raw))
                    {
                        if (!preg_match("/" . $instruction . "/u", $raw, $regexps))
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
        private string $raw;
        public string $instruction;
        public $splitted = null;

        public function __construct($raw, $splitted)
        {
            $this->raw = $raw;
            $this->instruction = strtoupper($splitted[1]);
            $this->splitted = $splitted;
        }

        public function get_instruction(): string
        {
            return "";
        }

        public function get_arguments($xml_dom, $node)
        {
            throw new Exception("Not implemented");
        }

        public function __toString(): string
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
            return '<arg1 type="var">GF@a</arg1>\n';
        }
    }

    class DoubleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            return '<arg1 type="var">GF@a</arg1>\n<arg2 type="var">GF@a</arg2>';
        }
    }

    class TripleArgsInstruction extends Instruction {
        public function get_arguments($xml_dom, $node)
        {
            return '<arg1 type="var">GF@a</arg1>\n<arg2 type="var">GF@a</arg2>\n<arg3 type="var">GF@a</arg3>';
        }
    }

    class Parser{
        private int $line_number = -1;
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

        function prepare_line($line)
        {
            $line = $this->remove_newline($line);
            return $this->remove_comments($line);
        }

        function parse_lines()
        {
            $instruction_parser = new InstructionParser();
            $instructions = new SplDoublyLinkedList();

            while($line = fgets(STDIN))
            {
                $line = $this->prepare_line($line);
                $this->line_number++;

                if (strlen($line) == 0)
                    continue;

                if (($this->line_number == 0 and $line != ".IPPcode22") or
                    ($this->line_number != 0 and $line == ".IPPcode22"))
                    $this->errors->error_exit($this->errors::INVALID_HEAD);
                elseif ($this->line_number == 0 && $line == ".IPPcode22")
                    continue;

                $parsed_instruction = $instruction_parser->get_instruction($line);
                $instructions->add($this->line_number - 1, $parsed_instruction);

                echo $parsed_instruction . "\n";
            }
            return $instructions;
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $parser = new Parser();
    $instructions = $parser->parse_lines();












