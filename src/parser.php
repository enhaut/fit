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

    class Parser{
        private int $line_number = 0;
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
            while($line = fgets(STDIN))
            {
                $line = $this->prepare_line($line);
                $this->line_number++;

                if (!strlen($line))
                    continue;

                if (($this->line_number == 0 and $line != ".IPPcode22") or
                    ($this->line_number != 0 and $line == ".IPPcode22"))
                    $this->errors->error_exit($this->errors::INVALID_HEAD);
                elseif ($this->line_number == 0 && $line == ".IPPcode22")
                    continue;


            }
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $parser = new Parser();
    $parser->parse_lines();












