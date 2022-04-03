<?PHP
    ini_set('display_errors', 'stderr');


    class ArgumentParser{
        private $argc;
        private $argv;

        public $directory;
        public $recursive;
        public $parse_script;
        public $int_script;
        public $parse_only;
        public $int_only;
        public $jexampath;
        public $nocleanup;

        /**
         * @param $argc int number of total arguments
         * @param $argv array array of arguments
         */
        public function __construct($argc, $argv)
        {
            $this->argc = $argc;
            $this->argv = $argv;

            $this->directory = null;
            $this->recursive = false;
            $this->parse_script = null;
            $this->int_script = null;
            $this->parse_only = false;
            $this->int_only = false;
            $this->jexampath = null;
            $this->nocleanup = false;
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
            echo "Usage: php test.php\n";
            echo "\t--directory=path\t\tpath to the test\n";
            echo "\t--recursive\t\t\trecursive looking up for tests\n";
            echo "\t--parse-script=file\t\tpath to the parse.php script\n";
            echo "\t--int-script=file\t\tpath to the interpret.py script\n";
            echo "\t--parse-only\t\t\ttest parser only\n";
            echo "\t--int-only\t\t\ttest interpret only\n";
            echo "\t--jexampath=path\t\tpath to the JExamXML binary\n";
            echo "\t--noclean\t\t\tdo not remove temporary files\n";

            exit($exit_code);
        }

        /**
         * @brief Function returns parsed value of argument.
         *
         * @param $value
         * @return string
         */
        function get_argument_value($value)
        {
            $value = substr($value, strpos($value, "=") + 1);
            $value = str_replace('"', '', $value);
            return trim($value);
        }

        /**
         * @brief Function checks for invalid arguments combinations.
         * @return void
         */
        function check_invalid_params_usage()
        {
            if (($this->parse_only and ($this->int_only or $this->int_script !== null)) or
                ($this->int_only and ($this->parse_only or $this->parse_script !== null)))
                $this->print_help(10);
        }

        /**
         * @brief Function sets default paths
         * @return void
         */
        function set_defaults()
        {
            if ($this->parse_script === null)
                $this->parse_script = "parse.php";
            if ($this->int_script === null)
                $this->int_script = "interpret.py";
            if ($this->jexampath === null)
                $this->jexampath = "/pub/courses/ipp/jexamxml/";
            if ($this->directory === null)
                $this->directory = ".";
        }

        /**
         * @brief Method parses program arguments.
         *
         * @return void
         */
        function parse()
        {
            for ($i = 1; $i < $this->argc; $i++)
                {
                    $argument = $this->argv[$i];
                    if (str_starts_with($argument, "--directory"))
                        $this->directory = $this->get_argument_value($argument);
                    elseif ($argument == "--recursive")
                        $this->recursive = true;
                    elseif (str_starts_with($argument, "--parse-script"))
                        $this->parse_script = $this->get_argument_value($argument);
                    elseif (str_starts_with($argument, "--int-script"))
                        $this->int_script = $this->get_argument_value($argument);
                    elseif ($argument == "--parse-only")
                        $this->parse_only = true;
                    elseif ($argument == "--int-only")
                        $this->int_only = true;
                    elseif (str_starts_with($argument, "--jexampath"))
                        $this->jexampath = $this->get_argument_value($argument);
                    elseif ($argument == "--noclean")
                        $this->nocleanup = true;
                    elseif ($argument == "--help")
                        $this->print_help(0);
                    else
                        $this->print_help(10);
                }
            $this->check_invalid_params_usage();
            $this->set_defaults();
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    echo "dir:".$arguments->directory."\n";
    echo "rec:".$arguments->recursive."\n";
    echo "parse scr:".$arguments->parse_script."\n";
    echo "int scr:".$arguments->int_script."\n";
    echo "parse only:".$arguments->parse_only."\n";
    echo "int only:".$arguments->int_only."\n";
    echo "jexam:".$arguments->jexampath."\n";
    echo "noclean:".$arguments->nocleanup."\n";