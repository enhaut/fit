<?PHP
    ini_set('display_errors', 'stderr');

    function error_exit($message, $exit_code)
    {
        echo $message . "\n";
        exit($exit_code);
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
                echo $this->line_number . ": " . $line . "\n";
            }
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $parser = new Parser();
    $parser->parse_lines();












