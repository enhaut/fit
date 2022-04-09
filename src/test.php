<?PHP
    ini_set('display_errors', 'stderr');


    class HTMLGenerator{
        private $output;

        public function __construct()
        {
            $this->output = "";
        }

        public function head()
        {
            $this->output .= "<html lang=\"en\"><meta charset=\"UTF-8\"><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">";
            $this->output .= "<style>
                                body {font-family: 'Segoe UI', Arial, 'Noto Sans', sans-serif;}
                                .dir{width:60%;margin-left: auto;margin-right: auto; background-color: darkgray; border-radius: 10px;padding-left: 1em;padding-bottom: .5em}
                                .test{width: 80%; margin-left: 3%}
                                p{margin-left: 7.5%;margin-top: -1em;background-color: peachpuff;padding: .3em;border-radius: 5px;max-height: 15em;overflow-y: scroll;}
                              </style>{SUMMARY}";
        }

        public function footer()
        {
            $this->output .= "</html>\n";
        }
        public function test_dir($directory, $results)
        {
            $this->output .= "<div class='dir'>";
            $this->output .= "<h1>".str_replace("//", "/", $directory)."</h1>";

            foreach ($results as $result)
            {
                $test_name = explode("/", $result->path);
                $this->test($test_name[count($test_name) - 1], $result->evaluated, $result->description);
            }
        }
        public function test($path, $result, $desc)
        {
            $passed_emoji = ($result) ? "✅ " : "❌ ";
            $this->output .= "<div class='test'> <h2 class='test'>". $passed_emoji . $path ."</h2>";
            if ($desc)
                $this->output .= "<p>". $desc ."</p>";

            $this->output .= "</div>";

        }

        public function summary($results)
        {
            $tests = count($results);
            $passed = 0;
            if ($tests > 0)
            {
                foreach ($results as $result)
                    if ($result->evaluated)
                        $passed++;
                $percentage =  round(($passed / $tests * 100));
            }else
                $percentage = 100;

            $summary = "<div class='dir' style='text-align: center'>";
            $summary .= "<h1>Summary</h1>";
            $summary .= "<h3>Tests: ". $tests;
            $summary .= "<br>Passed: ". $passed;
            $summary .= "<br>Failed: ". ($tests - $passed);
            $summary .= "</h3><h2 style='color:". (($percentage >= 50) ? "darkgreen" : "firebrick") ."'>Result: ". $percentage;
            $summary .= "%</h2></div>";

            $this->output = str_replace("{SUMMARY}", $summary, $this->output);
        }

        public function div_end()
        {
            $this->output .= "</div>";
        }

        public function save()
        {
            echo $this->output;
        }
    }
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
                $this->jexampath = "/pub/courses/ipp/jexamxml";
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

    class TestResult{
        public $output;
        public $code;
        public $evaluated;
        public $path;
        public $description;

        public function __construct($output, $code, $evaluated, $path, $desc)
        {
            $this->output = $output;
            $this->code = $code;
            $this->evaluated = $evaluated;
            $this->path = $path;
            $this->description = $desc;
        }
    }

    class Tester{
        private $args;
        private $generator;
        private $tmp_files;

        public function __construct($args)
        {
            $this->args = $args;
            $this->generator = new HTMLGenerator();
            $this->tmp_files = array();
        }

        private function get_test_dirs($directory)
        {
            if (!is_dir($directory))
                return array();

            $files = scandir($directory);
            $dirs = array($directory);

            if ($this->args->recursive)
                foreach ($files as $key => $subdir)
                {
                    if (!str_starts_with($subdir, "."))  // skip hidden directories
                    {
                        $file = $directory . DIRECTORY_SEPARATOR . $subdir;
                        if (is_dir($file))
                            $dirs = array_merge($dirs, $this->get_test_dirs($file));
                    }
                }
            return $dirs;
        }

        private function get_tests($directory)
        {
            $files = scandir($directory);
            $tests = array();

            foreach ($files as $key => $file)
            {
                $name = $directory . DIRECTORY_SEPARATOR . $file;
                if (str_ends_with($name, ".src"))
                    $tests[] = $name;
            }
            return $tests;
        }

        private function get_file($file)
        {
            try {
                if (!file_exists($file))
                {
                    $this->tmp_files[] = $file;
                    $created = fopen($file, 'w');
                    if (!$created)
                        exit(11);

                    if (str_ends_with($file, ".rc"))
                        fwrite($created, "0");

                    fclose($created);
                }
            }catch (Exception){
                exit(11);
            }
        }

        private function run_command($command)
        {
            $code = 0;
            $output = "";
            exec($command, $output, $code);

            return array($code, $output);
        }
        private function read_file($name)
        {
            $file = fopen($name, "r");
            if (!$file)
                exit(11);

            return fread($file, filesize($name));
        }

        private function evaluate($out, $test, $output, $xml_output=false)
        {
            $result = false;
            $desc = "";

            if ($out[0] != 0)
            {
                $rc_filename = str_replace(".src", ".rc", $test);
                $this->get_file($rc_filename);

                $rc = $this->read_file($rc_filename);
                if ($rc == $out[0])
                    $result = true;
                else
                    $desc = "Invalid exit code: ". $out[0] . " expected: ".$rc;
            }else{
                $out_filename = str_replace(".src", ".out", $test);
                $this->get_file($out_filename);

                if ($xml_output)
                {
                    $delta_filename = str_replace(".src", ".out_delta", $test);
                    $this->get_file($out_filename);

                    $command = "java -jar " . $this->args->jexampath . "/jexamxml.jar " . $output . " " . $out_filename . " " . $delta_filename . " " . $this->args->jexampath . "/options";
                }else
                    $command = "diff -u ".$out_filename." ".$output;

                $out = $this->run_command($command);

                if ($out[0] != 0)
                    $desc = implode('<br>', $out[1]);
                else
                    $result = true;
            }
            return array($result, $desc);
        }

        private function int_test($test, $source)
        {
            $input = str_replace(".src", ".in", $test);
            $this->get_file($input);

            $output = str_replace(".src", ".tmp_out", $test);
            if (!file_exists($output))
                $this->tmp_files[] = $output;

            $command = "cat ".$source." | python3.8 ". $this->args->int_script ." --input=".$input." &> ".$output;
            $out = $this->run_command($command);

            $evaluated = $this->evaluate($out, $test, $output);

            return new TestResult("", 0, $evaluated[0], $test, $evaluated[1]);
        }

        private function parse_test($file)
        {
            $input = str_replace(".src", ".in", $file);
            $this->get_file($input);

            $output = str_replace(".src", ".tmp_parse_out", $file);
            if (!file_exists($output))
                $this->tmp_files[] = $output;

            $command = "cat ".$file." | php8.1 ". $this->args->parse_script ." &> ".$output;
            $out = $this->run_command($command);

            $evaluated = array(true, "");
            if ($this->args->parse_only)
                $evaluated = $this->evaluate($out, $file, $output, true);
            else
                $evaluated[0] = ($out[0] == 0);

            return new TestResult("", 0, $evaluated[0], $output, $evaluated[1]);
        }

        private function run_test($file)
        {
            if (!$this->args->int_only and !$this->args->parse_only) // both tests
            {
                $result = $this->parse_test($file);
                if ($result->evaluated)
                    return $this->int_test($file, $result->path);

                return $result;
            }else if ($this->args->int_only)
                return $this->int_test($file, $file);
            else if ($this->args->parse_only)
                return $this->parse_test($file);

            return new TestResult("", 0, false, $file, "test");
        }

        private function test_dir($directory)
        {
            $tests = $this->get_tests($directory);
            $results = array();

            foreach ($tests as $key => $test)
            {
                $results[] = $this->run_test($test);
            }

            if (count($results) > 0)
            {
                $this->generator->test_dir($directory, $results);
                $this->generator->div_end();
            }
            return $results;
        }

        private function remove_tmp()
        {
            foreach ($this->tmp_files as $file)
            {
                try{
                    unlink($file);
                }catch (Exception)
                {
                    continue;
                }
            }
        }
        public function test()
        {
            $this->generator->head();
            $results = array();

            $dirs = $this->get_test_dirs($this->args->directory);
            foreach ($dirs as $key => $path)
                $results = array_merge($results, $this->test_dir($path));

            $this->generator->summary($results);
            $this->generator->footer();

            if (!$this->args->nocleanup)
                $this->remove_tmp();
        }

        public function save()
        {
            $this->generator->save();
        }
    }

    $arguments = new ArgumentParser($argc, $argv);
    $arguments->parse();

    $tester = new Tester($arguments);
    $tester->test();
    $tester->save();
