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
            0 => array("CREATEFRAME", "PUSHFRAME", "POPFRAME", "RETURN", "BREAK"),
            1 => array(
                "DEFVAR" . self::insParamsRegexp["var"],
                "CALL" . self::insParamsRegexp["label"],
                "PUSHS" . self::insParamsRegexp["symb"],
                "POPS" . self::insParamsRegexp["var"],
                "WRITE" . self::insParamsRegexp["symb"],
                "LABEL" . self::insParamsRegexp["label"],
                "JUMP" . self::insParamsRegexp["label"],
                "EXIT" . self::insParamsRegexp["symb"],
                "DPRINT" . self::insParamsRegexp["symb"]),
            2 => array(
                "MOVE" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"],
                "INT2CHAR" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"],
                "READ" . self::insParamsRegexp["var"] . self::insParamsRegexp["type"],
                "STRLEN" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"],
                "TYPE" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"]),
            3 => array(
                "ADD" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "SUB" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "MUL" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "IDIV" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "LT" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "GT" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "EQ" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "AND" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "OR" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "NOT" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "STRI2INT" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "CONCAT" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "GETCHAR" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "SETCHAR" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "JUMPIFEQ" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"],
                "JUMPIFNEQ" . self::insParamsRegexp["var"] . self::insParamsRegexp["symb"] . self::insParamsRegexp["symb"])
            );

        public function get_instruction($raw)
        {
            foreach (self::instructions as $instructionArrayIndex => $instructionArray)
            {
                foreach ($instructionArray as $instruction)
                {
                    $regexps = explode("\s+", $instruction, 3);
                    if (preg_match("/". $regexps[0] . "/u", $raw))
                    {
                        if (!preg_match("/" . $instruction . "/u", $raw))
                            exit(23);

                        switch ($instructionArrayIndex)
                        {
                            case 0:
                                return new NoArgsInstruction($raw);
                            case 1:
                                return new SingleArgsInstruction($raw);
                            case 2:
                                return new DoubleArgsInstruction($raw);
                            case 3:
                                return new TripleArgsInstruction($raw);
                        }
                    }
                }
            }
            exit(22);
        }
    }

    class Instruction {
        private string $raw;

        public function __construct($raw)
        {
            $this->raw = $raw;
        }

        public function get_instruction(): string
        {
            return "";
        }

        public function get_arguments(): string
        {
            throw new Exception("Not implemented");
        }

        public function __toString(): string
        {
            return $this->raw;
        }
    }

    class NoArgsInstruction extends Instruction {
        public function get_arguments(): string
        {
            return "";
        }
    }

    class SingleArgsInstruction extends Instruction {
        public function get_arguments(): string
        {
            return '<arg1 type="var">GF@a</arg1>\n';
        }
    }

    class DoubleArgsInstruction extends Instruction {
        public function get_arguments(): string
        {
            return '<arg1 type="var">GF@a</arg1>\n<arg2 type="var">GF@a</arg2>';
        }
    }

    class TripleArgsInstruction extends Instruction {
        public function get_arguments(): string
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
                echo $parsed_instruction . "\n";
            }
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $parser = new Parser();
    $parser->parse_lines();












