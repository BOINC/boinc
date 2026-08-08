<?php // -*- php -*-

// test.inc
//
// A set of classes for testing BOINC.
// These classes let you create multiple projects and multiple hosts
// (all running on a single machine),
// add applications and work units, run the system,
// and verify that the results are correct.
//
// See doc/test.html for details

include_once("version.inc");
include_once("boinc_db.inc");

ob_end_flush();         // let us see what's going on

// Create a handler function
function my_assert_handler ($file, $line, $code) {
    echo "Assertion Failed:
        File '$file'
        Line '$line'
        Code '$code'
";
    exit(1);
}

assert_options(ASSERT_ACTIVE  , 1);
assert_options(ASSERT_WARNING , 1);
assert_options(ASSERT_BAIL    , 1);
assert_options(ASSERT_CALLBACK, 'my_assert_handler');

$errors = 0;

// get an environment variable, and abort script if missing
//
function get_env_var($name, $defvalue = -1) {
    $x = getenv($name);
    if ($x == null) {
        if ($defvalue != -1) {
            return $defvalue;
        } else {
            echo "Environment variable $name not defined\n";
            exit();
        }
    }
    return $x;
}

/// VERBOSITY LEVELS:
//    VERBOSE=0  print nothing
//    VERBOSE=1  show only up to 1 line
//    VERBOSE=2  print everything

define("VERBOSE", get_env_var("TEST_VERBOSE",1));

function verbose_echo($level, $line)
{
    if ($level == 0) {
        if (VERBOSE == 1) {
            echo "\n";
        }
        echo "$line\n";
    } else if (VERBOSE >= $level) {
        if (VERBOSE == 2) {
            echo "$line\n";
        } else if (VERBOSE == 1) {
            echo "\r                                                                               ";
            echo "\r$line ";
        }
    }
}

function error($msg, $fatal=0)
{
    global $errors;
    ++$errors;
    verbose_echo(0, "ERROR: $msg");
    if ($fatal) {
        exit(1);
    }
}

// when sleeping for a long time this shows sleep progress
function verbose_sleep($msg, $wait)
{
    $front = "$msg [sleep ";
    $back = "]";
    for ($i = 1; $i <= $wait; ++$i) {
        $m = $front . str_repeat('.',$i) . str_repeat(' ',$wait-$i) . $back;
        verbose_echo(1, $m);
        sleep(1);
    }
}

function XPassThru($cmd, $failok=0)
{
    PassThru($cmd, $retval);
    if (!$failok && $retval) {
        verbose_echo(0, "Command failed: $cmd");
        exit(1);
    }
}

function verbose_XPassThru($cmd, $failok=0)
{
    verbose_echo(2,"   $cmd");
    return XPassThru($cmd, $failok);
}

function proxerize($s, $t)
{
    if ($t) {
        return preg_replace(",http://[^/]*/,", "http://localhost:8080/", $s);
    } else {
        return $s;
    }
}

define("KEY_DIR"      , get_env_var("BOINC_KEY_DIR"));
define("SHMEM_KEY"    , get_env_var("BOINC_SHMEM_KEY"));
define("PROJECTS_DIR" , get_env_var("BOINC_PROJECTS_DIR"));
define("CGI_URL"      , proxerize(get_env_var("BOINC_CGI_URL"), $use_proxy_cgi));
define("HTML_URL"     , proxerize(get_env_var("BOINC_HTML_URL"), $use_proxy_html));
define("USER_NAME"    , get_env_var("BOINC_USER_NAME", get_env_var("USER","")));
define("CGI_DIR"      , get_env_var("BOINC_CGI_DIR"));
define("HTML_DIR"     , get_env_var("BOINC_HTML_DIR"));
define("HOSTS_DIR"    , get_env_var("BOINC_HOSTS_DIR"));

function check_program_exists($prog) {
    if (!is_executable($prog)) {
        verbose_echo(0, "Executable not found: $prog");
        verbose_echo(0, "Did you `make' yet?\n");
        die(1);
    }
}


function check_core_client_executable()
{
    check_program_exists(SRC_DIR."/client/".CLIENT_BIN_FILENAME);
}

function check_app_executable($app)
{
    check_program_exists(SRC_DIR."/apps/".$app);
}

function run_db_script($script, $db_name) {
    $db_dir = SRC_DIR . "/db";
    $x = "sed -e s/BOINC_DB_NAME/$db_name/ $db_dir/$script | mysql";
    // echo $x;
    XPassThru($x);
}

// expand a macro in a file
//
function macro_substitute($macro, $replacement, $infile, $outfile) {
    $x = "sed -e s/$macro/$replacement/ $infile > $outfile";
    //echo $x;
    XPassThru($x);
}

// make a file executable
//
function make_executable($name) {
    XPassThru("chmod uog+x $name");
}

// given a project URL,
// return the name of the account file
//
function account_file_name($url) {
    $x = strstr($url, "http://");
    if ($x) {
        $x = substr($x, strlen("http://"));
    } else {
        $x = $url;
    }
    $encoded_name = strtr($x, "/", "_");
    $encoded_name = substr($encoded_name, 0,strlen($encoded_name)-1);
    return "account_".$encoded_name.".xml";
}

function db_open($db_name) {
    $retval = mysql_connect();
    if (!$retval) {
        echo "mysql_connect() failed\n";
        exit();
    }
    $retval = mysql_select_db($db_name);
    if (!$retval) {
        echo "mysql_select_db() failed\n";
        exit();
    }
}

function db_query($query) {
    if (!mysql_query($query)) {
        echo "mysql_query failed: $query\n";
        echo mysql_error();
        exit();
    }
}

function run_tool($cmd) {
    verbose_XPassThru(SRC_DIR."/tools/".$cmd);
}

function create_keys() {
    $key_dir = KEY_DIR;
    $lib_dir = SRC_DIR."/lib";
    XPassThru("$lib_dir/crypt_prog -genkey 1024 $key_dir/upload_private $key_dir/upload_public");
    XPassThru("$lib_dir/crypt_prog -genkey 1024 $key_dir/code_sign_private $key_dir/code_sign_public");
}

function check_exists($file) {
    if (!file_exists($file)) {
        error("file doesn't exist: $file\n");
    }
}

function check_deleted($file) {
    if (file_exists($file)) {
        error("file wasn't deleted: $file\n");
    }
}

class Platform {
    var $name;
    var $user_friendly_name;

    function Platform($name, $ufn) {
        $this->name = $name;
        $this->user_friendly_name = $ufn;
    }
}

class Core_Version {
    var $version;
    var $exec_dir;
    var $exec_name;
    var $platform;
    function Core_Version() {
        $this->version   = 1;
        $this->platform  = new Platform(PLATFORM, PLATFORM);
        $this->exec_dir  = SRC_DIR . "/client";
        $this->exec_name = CLIENT_BIN_FILENAME;
    }
}

class App {
    var $name;

    function App($name) {
        $this->name = $name;
    }
}

class App_Version {
    var $app;
    var $version;
    var $exec_dir;
    var $exec_names;    // array of filenames;
                        // defaults to one element: $app->name
    var $platform;

    function App_Version($app) {
        $this->exec_names    = array();
        $this->exec_dir      = SRC_DIR . "/apps";
        $this->exec_names[0] = $app->name;
        $this->app           = $app;
        $this->version       = 1;
        $this->platform      = new Platform(PLATFORM, PLATFORM);
    }
}

class Project {
    var $short_name;
    var $long_name;
    var $users;
    var $core_versions;
    var $apps;
    var $app_versions;
    var $platforms;
    var $project_dir;
    var $db_name;
    var $db_passwd;
    var $generate_keys;
    var $shmem_key;
    var $key_dir;
    var $download_url;
    var $scheduler_url;
    var $upload_url;
    var $user_name;
    var $master_url;
    var $resource_share;
    var $source_dir;
    var $project_php_file;
    var $project_prefs_php_file;

    function Project() {
        $this->short_name = "test";
        $this->long_name = "test";
        $this->users = array();
        $this->core_versions = array();
        $this->apps = array();
        $this->app_versions = array();
        $this->platforms = array();
        $this->db_passwd = "";
        $this->generate_keys = false;
        $this->shmem_key = SHMEM_KEY;
        $this->resource_share = 1;
        $this->source_dir = SRC_DIR;
    }

    function add_user($user) {
        array_push($this->users, $user);
    }

    function add_core_version($core_version) {
        array_push($this->core_versions, $core_version);
    }

    function add_app($app) {
        array_push($this->apps, $app);
    }

    function add_app_version($app_version) {
        array_push($this->app_versions, $app_version);
    }

    function add_platform($platform) {
        array_push($this->platforms, $platform);
    }

    function add_app_and_version($appname) {
        $app = new App($appname);
        $app_version = new App_Version($app);
        $this->add_app($app);
        $this->add_app_version($app_version);
    }

    function add_core_and_version() {
        $this->add_core_version(new Core_Version());
    }

    function mkdir($dir, $chmod=0)
    {
        $d = "$this->project_dir/$dir";
        mkdir($d,0775);
        if ($chmod) {
            chmod($d, 0775);
        }
    }

    function copy($source, $dest, $failok=0)
    {
        XPassThru("cp -f $this->source_dir/$source $this->project_dir/$dest >> /dev/null 2>&1", $failok);
    }

    // Set up the database and directory structures for a project
    //
    function install($scheduler_file = null) {
        $base_dir = PROJECTS_DIR;
        $cgi_url = CGI_URL."/".$this->short_name;
        $this->download_url = HTML_URL."/".$this->short_name."/download";
        $this->upload_url = $cgi_url."/file_upload_handler";
        $this->scheduler_url = $cgi_url."/cgi";
        $this->project_dir = $base_dir."/".$this->short_name;
        $this->master_url = HTML_URL."/".$this->short_name."/";

        verbose_echo(1, "Deleting previous test runs");
        XPassThru("rm -rf $this->project_dir");

        verbose_echo(1, "Creating server directories");

        // make the CGI writeable in case scheduler writes req/reply files
        $this->mkdir("");
        $this->mkdir("cgi", 1);
        $this->mkdir("upload", 1);
        $this->mkdir("download");
        $this->mkdir("keys");
        $this->mkdir("html_ops");
        $this->mkdir("html_user");
        XPassThru("ln -s $this->project_dir/download $this->project_dir/html_user/download");

        if ($this->generate_keys) {
            echo "KEY GENERATION NOT IMPLEMENTED YET\n";
        } else {
            $this->key_dir = KEY_DIR;
            XPassThru("cp $this->key_dir/* $this->project_dir/keys");
        }

        // set up the database
        //
        verbose_echo(1, "Setting up database");
        $this->user_name = USER_NAME;
        if (!$this->db_name)
            $this->db_name = $this->user_name."_".$this->short_name;
        run_db_script("drop.sql", $this->db_name);
        run_db_script("schema.sql", $this->db_name);
        run_db_script("constraints.sql", $this->db_name);

        db_open($this->db_name);
        db_query("insert into project(short_name, long_name) values('$this->short_name', '$this->long_name')");

        for ($i=0; $i<sizeof($this->users); $i++) {
            $user = $this->users[$i];
            $now = time(0);
            $pp = null;
            if ($user->project_prefs) {
                $pp = "<project_preferences>\n$user->project_prefs</project_preferences>\n";
            }
            $gp = null;
            if ($user->global_prefs) {
                $gp = "<global_preferences>\n$user->global_prefs</global_preferences>\n";
            }
            db_query("insert into user values (0, $now, '$user->email_addr', '$user->name', '$user->authenticator', 'Peru', '12345', 0, 0, 0, '$gp', '$pp', 0, 'home', '', 0, 1)");
        }

        $napp = sizeof($this->apps);
        verbose_echo(1, "Adding $napp app(s) to database");
        for ($i=0; $i<$napp; $i++) {
            $app = $this->apps[$i];
            $now = time(0);
            check_app_executable($app->name);
            db_query("insert into app(name, create_time) values ('$app->name', $now)");
        }

        verbose_echo(1, "Adding platforms(s) to database");
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app_version = $this->app_versions[$i];
            $p = $app_version->platform;
            $found = false;
            for ($j=0; $j<sizeof($this->platforms); $j++) {
                if (strcmp($this->platforms[$j]->name, $p->name) == 0) {
                    $found = true;
                    break;
                }
            }
            if (!$found) {
                array_push($this->platforms, $app_version->platform);
            }
        }
        for ($i=0; $i<sizeof($this->platforms); $i++) {
            $platform = $this->platforms[$i];
            run_tool("add platform -db_name $this->db_name -platform_name $platform->name -user_friendly_name '$platform->user_friendly_name'");
        }

        $ncore = sizeof($this->core_versions);
        verbose_echo(1, "Adding $ncore core version(s) to database");
        for ($i=0; $i<$ncore; $i++) {
            $core_version = $this->core_versions[$i];
            $p = $core_version->platform;
            $x = "add core_version -db_name $this->db_name -platform_name $p->name -version $core_version->version -download_dir $this->project_dir/download -download_url $this->download_url -exec_dir $core_version->exec_dir -exec_files $core_version->exec_name";
            run_tool($x);
        }

        $nappv = sizeof($this->app_versions);
        verbose_echo(1, "Adding $nappv app version(s) to database");
        for ($i=0; $i<$nappv; $i++) {
            $app_version = $this->app_versions[$i];
            $app = $app_version->app;
            $p = $app_version->platform;
            $x = "add app_version -db_name $this->db_name -app_name '$app->name' -platform_name $p->name -version $app_version->version -download_dir $this->project_dir/download -download_url $this->download_url -code_sign_keyfile $this->key_dir/code_sign_private -exec_dir $app_version->exec_dir -exec_files";
            for ($j=0; $j<sizeof($app_version->exec_names); $j++) {
                $x = $x." ".$app_version->exec_names[$j];
            }
            run_tool($x);
        }

        verbose_echo(1, "Creating html directories");
        // copy the user and administrative PHP files to the project dir,
        //
        $this->copy('html_user/*.php'      , 'html_user/');
        $this->copy('html_user/*.inc'      , 'html_user/');
        $this->copy('html_user/*.html'     , 'html_user/');
        $this->copy('html_ops/*.php'       , 'html_ops/');
        $this->copy('html_ops/*.inc'       , 'html_ops/');
        $this->copy('tools/country_select' , 'html_user/');
        if ($this->project_php_file) {
            $this->copy('html_user/$this->project_php_file', 'html_user/project.inc');
        }
        if ($this->project_prefs_php_file) {
            $this->copy('html_user/$this->project_prefs_php_file', 'html_user/project_specific_prefs.inc');
        }

        // Copy the sched server in the cgi directory with the
        // cgi names given $source_dir/html_usr/schedulers.txt
        //
        verbose_echo(1, "Copying cgi programs");
        if ($scheduler_file == null) {
            $scheduler_file = "schedulers.txt";
            $f = fopen("$this->project_dir/html_user/schedulers.txt", "w");
            fputs($f,"<scheduler>".$this->scheduler_url."</scheduler>\n");
            fclose($f);
        } else {
            $f = fopen("$this->project_dir/html_user/$scheduler_file", "r");
            while (true) {
                $scheduler_url = fgets($f, 1000);
                if($scheduler_url == false) break;
                $temp = substr($scheduler_url, 0, strrpos($scheduler_url, '<'));
                $cgi_name = substr($temp, strrpos($temp, '/')+1, strlen($temp) - strrpos($temp, '/'));
                $this->copy('sched/cgi', 'cgi/$cgi_name');
            }

            fclose($f);
        }

        // copy all the backend programs to the CGI directory
        //
        $this->copy('sched/cgi'                 , 'cgi/');
        $this->copy('sched/file_upload_handler' , 'cgi/');
        $this->copy('sched/make_work'           , 'cgi/');
        $this->copy('sched/feeder'              , 'cgi/');
        $this->copy('sched/timeout_check'       , 'cgi/');
        $this->copy('sched/validate_test'       , 'cgi/');
        $this->copy('sched/file_deleter'        , 'cgi/');
        $this->copy('sched/assimilator'         , 'cgi/');
        $this->copy('sched/start_servers'       , 'cgi/');

        verbose_echo(1, "Writing config files");
        // write the server config file
        //
        $this->append_config("<config>\n" .
            "<db_name>$this->db_name</db_name>\n" .
            "<db_passwd>$this->db_passwd</db_passwd>\n" .
            "<shmem_key>$this->shmem_key</shmem_key>\n" .
            "<key_dir>$this->key_dir</key_dir>\n" .
            "<download_url>$this->download_url</download_url>\n" .
            "<download_dir>$this->project_dir/download</download_dir>\n" .
            "<upload_url>$this->upload_url</upload_url>\n" .
            "<upload_dir>$this->project_dir/upload</upload_dir>\n" .
            "<user_name>$this->user_name</user_name>\n" .
            "</config>\n");

        // put a file with the database name and other info
        // in each HTML directory
        //
        $f = fopen("$this->project_dir/html_user/.htconfig.xml", "w");
        fputs($f, "<db_name>$this->db_name</db_name>\n");
        fputs($f, "<db_passwd>$this->db_name</db_passwd>\n");
        fputs($f, "<download_url>$this->download_url</download_url>\n");
        fputs($f, "<cgi_url>$cgi_url</cgi_url>\n");
        fclose($f);
        XPassThru("cp $this->project_dir/html_user/.htconfig.xml $this->project_dir/html_ops");

        // edit "index.php" in the user HTML directory to have
        // the right file as the source for scheduler_urls;
        // default is schedulers.txt
        //
        $x = "sed -e s/FILE_NAME/$scheduler_file/ $this->project_dir/html_user/index.php > temp; mv temp $this->project_dir/html_user/index.php";
        XPassThru($x);

        verbose_echo(1, "Linking cgi programs");
        // create symbolic links to the CGI and HTML directories
        //
        $cgi_dir = CGI_DIR;
        $cgi_url = CGI_URL;
        $html_dir = HTML_DIR;
        $html_url = HTML_URL;
        XPassThru("rm -f $cgi_dir/$this->short_name");
        XPassThru("ln -s $this->project_dir/cgi $cgi_dir/$this->short_name");
        XPassThru("rm -f $html_dir/$this->short_name");
        XPassThru("ln -s $this->project_dir/html_user $html_dir/$this->short_name");
        $x = "ln -s $this->project_dir/html_ops ".$html_dir."/".$this->short_name."_admin";
        XPassThru($x);

        // show the URLs for user and admin sites
        //
        $admin_url = $html_url."/".$this->short_name."_admin/index.php";
        verbose_echo(2, "Master URL: $this->master_url");
        verbose_echo(2, "Admin URL:  $admin_url");
    }

    // Adds http password protection to the html_ops directory
    //
    function http_password($user,$password) {
        $f = fopen($this->project_dir."/html_ops/.htaccess", "w");
        fputs($f,"AuthName \"$this->long_name Administration\"\n");
        fputs($f,"AuthType Basic\n");
        fputs($f,"AuthUserFile $this->project_dir/html_ops/.htpasswd\n\n");
        fputs($f,"require valid-user\n");
        fclose($f);

        XPassThru("htpasswd -bc $this->project_dir/html_ops/.htpasswd $user $password");
    }

    // moves the master web page to temp
    // This is used to test exponential backoff on the client side.
    //
    function delete_masterindex() {
        XPassThru("mv $this->project_dir/html_user/index.php $this->project_dir/html_user/temp");
    }

    // moves temp back to the master web page
    // This is used to test exponential backoff on the client side.
    //
    function reestablish_masterindex() {
        XPassThru("mv $this->project_dir/html_user/temp $this->project_dir/html_user/index.php");

    }

    // delete the sched server for this project
    // This is used to test exponential backoff on the client side.
    //
    function delete_scheduler($cgi_num = null) {
        XPassThru("rm $this->project_dir/cgi/cgi$cgi_num");
    }

    // copies the sched server back into the CGI directory.
    // This is used to test exponential backoff on the client side.
    //
    function reinstall_scheduler($cgi_num=null) {
        XPassThru("cp $this->source_dir/sched/cgi $this->project_dir/cgi/cgi$cgi_num");
    }

    // moves the download directory to temp.
    // This is used to test exponential backoff
    //
    function delete_downloaddir($download_dir_num = null) {
        XPassThru("mv $this->project_dir/download$download_dir_num $this->project_dir/download_moved$download_dir_num");

    }

    // reinstalls the download directory.
    // This is used to test exponential backoff
    //
    function reinstall_downloaddir($download_dir_num = null) {
        XPassThru("mv $this->project_dir/download_moved$download_dir_num $this->project_dir/download$download_dir_num");
    }

    function remove_file_upload_handler($handler_num = null) {
        XPassThru("rm $this->project_dir/cgi/file_upload_handler$handler_num");
    }

    function reinstall_file_upload_handler($handler_num = null) {
        XPassThru("cp $this->source_dir/sched/file_upload_handler $this->project_dir/cgi/file_upload_handler$handler_num");
    }

    function start_servers() {
        XPassThru("cd $this->project_dir/cgi; ./start_servers >> start_servers.out 2>&1");
        verbose_sleep("Starting servers for project $this->short_name", 1);
    }

    function install_feeder() {
        $this->append_config("<start>./feeder -d 3 -asynch >> feeder.out 2>&1</start>\n");
    }

    function install_timeout_check($app, $nerror = 5,$ndet = 5, $nredundancy = 5){
        $this->append_config("<start>./timeout_check -d 3 -app $app->name -nerror $nerror -ndet $ndet -nredundancy $nredundancy -asynch >> timeout_check.out 2>&1</start>\n");
    }

    function install_make_work($work,$cushion,$redundancy){
        $result_template_path = realpath($work->result_template);
        $this->append_config("<start>./make_work -asynch -d 3 -cushion $cushion -redundancy $redundancy -result_template $result_template_path -wu_name $work->wu_template >> make_work.out 2>&1</start>\n");
    }

    function deinstall_make_work() {
        $this->remove_config("make_work");
    }

    // run the validator asynchronously
    //
    function install_validate($app, $quorum) {
        assert($quorum);
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app = $this->app_versions[$i]->app;
            assert($app);
            $this->append_config("<start>./validate_test -asynch -d 3 -app $app->name -quorum $quorum >> validate.out 2>&1</start>\n");
        }
    }

    // do one pass of validation
    //
    function validate($quorum) {
        assert($quorum);
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app = $this->app_versions[$i]->app;
            assert($app);
            verbose_echo(1, "Validating $app->name");
            XPassThru("cd $this->project_dir/cgi; ./validate_test -d 3 -one_pass -app $app->name -quorum $quorum >> validate.out 2>&1");
        }
    }

    function install_file_delete(){
        $this->append_config("<start>./file_deleter -d 3 -asynch >> file_deleter.out 2>&1</start>\n");
    }

    // do one pass of file_deleter
    //
    function file_delete() {
        XPassThru("cd $this->project_dir/cgi; ./file_deleter -d 3 -one_pass >> file_deleter.out 2>&1");
    }

    function install_assimilator() {
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app = $this->app_versions[$i]->app;
            $this->append_config("<start>./assimilator -asynch -app -d 3 $app->name >> assimilator.out 2>&1</start>\n");
        }
    }

    // do one pass of assimilator
    //
    function assimilate() {
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app = $this->app_versions[$i]->app;
            verbose_echo(1, "Assimilating $app->name");
            XPassThru("cd $this->project_dir/cgi; ./assimilator -one_pass -d 3 -app $app->name >> assimilator.out 2>&1");
        }
    }

    // start collecting data for stripcharts
    //
    function start_stripchart() {
        $source_dir = $this->source_dir;
        XPassThru("cp $source_dir/stripchart/stripchart.cgi $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/stripchart $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/stripchart.cnf $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/samples/looper $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/samples/db_looper $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/samples/datafiles $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/samples/get_load $this->project_dir/cgi/");
        XPassThru("cp $source_dir/stripchart/samples/dir_size $this->project_dir/cgi/");
        macro_substitute("BOINC_DB_NAME", $this->db_name, "$source_dir/stripchart/samples/db_count", "$this->project_dir/cgi/db_count");
        make_executable("$this->project_dir/cgi/db_count");
        XPassThru("cd $this->project_dir/cgi; looper get_load 1 > get_load_out &");
        XPassThru("cd $this->project_dir/cgi; db_looper 'result' 1 > count_results_out &");
        XPassThru("cd $this->project_dir/cgi; db_looper 'workunit where assimilate_state=2' 1 > assimilated_wus_out &");
        XPassThru("cd $this->project_dir/cgi; looper 'dir_size ../download' 1 > download_size_out &");
        XPassThru("cd $this->project_dir/cgi; looper 'dir_size ../upload' 1 > upload_size_out &");
    }

    // this should stop the feeder and any other daemons
    //
    function stop($nosleep=0) {
        $f = fopen($this->project_dir."/cgi/stop_server", "w");
        fputs($f, "<quit/>\n");
        fclose($f);
        // need to sleep because the feeder sleeps (up to 5+5+1) seconds to
        // check triggers.
        $sleep = $nosleep ? 0 : 7;
        verbose_sleep("Stopping server(s) for project $this->short_name", $sleep);
    }

    // append the line to the config file
    function append_config($line) {
        touch($this->project_dir."/cgi/.htconfig.xml");
        $f_old = fopen($this->project_dir."/cgi/.htconfig.xml", "r");
        $f_new = fopen($this->project_dir."/cgi/.htconfig.tmp", "w");
        $appended = false;
        while(!feof($f_old)) {
            $in_line = fgets($f_old,1024);
            if (strstr($in_line, "</config>")) {
                fputs($f_new, $line);
                $appended = true;
            }
            fputs($f_new, $in_line);
        }
        if (!$appended) fputs($f_new, $line);
        fclose($f_old);
        fclose($f_new);
        rename($this->project_dir."/cgi/.htconfig.tmp", $this->project_dir."/cgi/.htconfig.xml");
    }

    // remove any line containing the pattern from the config file
    function remove_config($pattern) {
        touch($this->project_dir."/cgi/.htconfig.xml");
        $f_old = fopen($this->project_dir."/cgi/.htconfig.xml", "r");
        $f_new = fopen($this->project_dir."/cgi/.htconfig.tmp", "w");
        while(!feof($f_old)) {
            $in_line = fgets($f_old, 1024);
            if (!strstr($in_line, $pattern))
                fputs($f_new, $in_line);
        }
        fclose($f_old);
        fclose($f_new);
        rename($this->project_dir."/cgi/.htconfig.tmp", $this->project_dir."/cgi/.htconfig.xml");
    }

    // remove the stop_server trigger
    //
    function restart() {
        unlink($this->project_dir."/cgi/stop_server");
    }

    function check_results($ntarget, $matchresult) {
        $n = 0;
        db_open($this->db_name);
        $result = mysql_query("select * from result");
        while ($x = mysql_fetch_object($result)) {
            $n++;
            if ($matchresult->server_state != null && $matchresult->server_state != $x->server_state) {
                error("result $x->id: unexpected state $x->server_state (expected $matchresult->server_state)");
            }
            if ($matchresult->stderr_out != null) {
                // need to use === to differentiate FALSE from 0. php sucks.
                if (strpos($x->stderr_out,$matchresult->stderr_out)===FALSE) {
                    error("result $x->id: unexpected stderr_out:

$x->stderr_out

expected:

$matchresult->stderr_out
");
                }
            }
            if ($matchresult->exit_state != null && $matchresult->exit_status != $x->exit_status) {
                error("result $x->id: unexpected exit_status $x->exit_status (expected $matchresult->exit_status)");
            }
        }
        if ($n != $ntarget) {
            error("expected $ntarget results, found $n.\n");
        }
    }

    function num_wus_left() {
        db_open($this->db_name);
        $result = mysql_query("select count(*) as cnt from result where server_state=2");
        $count = mysql_fetch_object($result);
        return $count->cnt;
    }

    function num_results_done() {
        db_open($this->db_name);
        $result = mysql_query("select count(*) as cnt from result where server_state=4");
        $count = mysql_fetch_object($result);
        return $count->cnt;
    }

    function compare_file($result, $correct) {
        PassThru("diff $this->project_dir/upload/$result $correct", $retval);
        if ($retval) {
            error("File mismatch for project $this->short_name: $result $correct");
        } else {
            verbose_echo(2, "Files match for project $this->short_name: $result $correct");
        }
    }

    function check_server_deleted($file) {
        check_deleted("$this->project_dir/" . $file);
    }

    function check_server_exists($file) {
        check_exists("$this->project_dir/" . $file);
    }

}

// represents an account on a particular project
//
class User {
    var $name;
    var $email_addr;
    var $authenticator;
    var $project_prefs;
    var $project;

    function User() {
        $this->name = "John";
        $this->email_addr = "john@boinc.org";
        $this->authenticator = "3f7b90793a0175ad0bda68684e8bd136";
    }
}


class Host {
    var $name;
    var $users;
    var $projects;
    var $host_dir;
    var $global_prefs;
    var $log_flags;

    function Host() {
        $this->name = "test";
        $this->users = array();
        $this->projects = array();
        $this->global_prefs = null;
        //$this->log_flags = null;
        $this->log_flags = "log_flags.xml";
    }

    function add_user($user, $project) {
        array_push($this->users, $user);
        array_push($this->projects, $project);
    }

    function install() {
        $base_dir = HOSTS_DIR;
        $this->host_dir = $base_dir."/".$this->name;
        XPassThru("rm -rf $this->host_dir");
        XPassThru("mkdir $this->host_dir");

        // create account files
        //
        verbose_echo(1, "Creating account files");
        for ($i=0; $i<sizeof($this->users); $i++) {
            $user = $this->users[$i];
            $project = $this->projects[$i];
            $filename = account_file_name($project->master_url);
            verbose_echo(2, "Writing $this->host_dir/$filename");
            $f = fopen($this->host_dir."/".$filename, "w");
            fputs($f, "<account>\n");
            fputs($f, "    <master_url>$project->master_url</master_url>\n");
            fputs($f, "    <authenticator>$user->authenticator</authenticator>\n");
            fputs($f, $user->project_prefs);
            fputs($f, "</account>\n");
            fclose($f);
        }

        // copy log flags, if any
        //
        if ($this->log_flags != null) {
            XPassThru("cp $this->log_flags $this->host_dir/log_flags.xml");
        }

        // copy global prefs, if any
        //
        if ($this->global_prefs != null) {
            XPassThru("cp $this->global_prefs.xml $this->host_dir/global_prefs.xml");
        }
    }

    function kill($boinc_pid) {
        if ($boinc_pid) {
            XPassThru("kill -9 $boinc_pid");
        }
    }

    // forks a new process to run this host with the given $args.
    // the pid returned is the pid of the newly forked php process.
    // NOTE: to kill the newly started host,
    // get_new_boincpid() must be called and kill() must be called on it.
    //
    function run_asynch($args) {
        // TODO: just use shell & to background?
        $pid = pcntl_fork();
        if ($pid == -1) {
            return -1;
        } else if ($pid) {
            return $pid; // we are the parent
        } else { //we are the child
            verbose_echo(1, "Running core client asynch");
            $source_dir = SRC_DIR;
            $exec_name = CLIENT_BIN_FILENAME;
            assert($source_dir && 1);
            assert($exec_name && 1);
            assert($this->host_dir && 1);
            verbose_XPassThru("cd $this->host_dir; $source_dir/client/$exec_name $args > client.out");
            exit(0);
        }
    }

    //returns a pid for a boinc process running in the system
    // that is different from $client_pid.
    // This call blocks until such process is started.
    //
    function get_new_client_pid($client_pid) {
        while(true) {
            $pid = exec("pgrep -n boinc");
            if(($pid != null) && ($pid != $client_pid)) {
                return $pid;
            }
        }
    }

    function run($args=null) {
        verbose_echo(1, "Running core client");
        $source_dir = SRC_DIR;
        $exec_name = CLIENT_BIN_FILENAME;
        assert($source_dir && 1);
        assert($exec_name && 1);
        assert($this->host_dir && 1);
        verbose_XPassThru("cd $this->host_dir; $source_dir/client/$exec_name $args > client.out", $ret);
        assert ($ret == 0);
    }

    // read a CPU time file written by the client.
    // This is sort of a kludge.
    //
    function read_cpu_time_file($file_name) {
        $time_file = fopen($this->host_dir/$file_name, "r");
        if($time_file == NULL) return 0;
        fscanf($time_file, "%f", $app_time);
        fclose($f);
        return $app_time;
    }

    function check_file_present($project, $filename) {
        $enc_url = str_replace("http://", "", $project->master_url);
        $enc_url = str_replace("/", "_", $enc_url);
        $path= "$this->host_dir/projects/$enc_url/$filename";
        if (!file_exists($path)) {
            error("file $path doesn't exist");
        }
    }
}

class Work {
    var $app;
    var $wu_template;
    var $result_template;
    var $redundancy;
    var $input_files;
    var $rsc_iops;
    var $rsc_fpops;
    var $rsc_memory;
    var $rsc_disk;
    var $delay_bound;

    function Work() {
        $this->input_files = array();
        $this->rsc_iops = 1.8e12;
        $this->rsc_fpops = 1e13;
        $this->rsc_memory = 1e7;
        $this->rsc_disk = 1e7;
        $this->delay_bound = 1000;
        $this->redundancy = 1;
    }

    function install($project) {
        verbose_echo(1, "Installing work <$this->wu_template> in project $project->short_name");
        if (!$this->app) {
            $this->app = $project->app_versions[0]->app;
        }
        $app = $this->app;
        assert($app);

        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            XPassThru("cp $x $project->project_dir/download");
        }

        // simulate multiple data servers by making symbolic links
        // to the download directory
        //
        $f = fopen($this->wu_template,"r");
        while(true) {
            $temp = fgets($f,1000);
            if($temp == false) break;
            $temp = stristr($temp,"<DOWNLOAD_URL/>");
            if($temp) {
                $pos = strpos($temp,">");
                if($temp[$pos + 2] != "<") {
                    $append = substr($temp, $pos+1,strpos($temp,"/<") - $pos -1);
                }
                XPassThru("ln -s $project->project_dir/download $project->project_dir/download$append");
            }
        }

        // simulate multiple data servers by making copies of
        // the file upload handler
        //
        fclose($f);
        $source_dir = SRC_DIR;
        $append = null;
        $f = fopen($this->result_template,"r");
        while(true) {
            $temp = fgets($f,1000);
            if($temp == false) break;
            $temp = stristr($temp,"<url>");
            if($temp) {
                $upload_url = strip_tags($temp,"<UPLOAD_URL/>");

                if(strip_tags($upload_url)) {
                    $append = strip_tags($upload_url);

                    XPassThru("cp $source_dir/sched/file_upload_handler $project->project_dir/cgi/file_upload_handler$append");

                }
            }
        }
        fclose($f);

        $cmd = "create_work -db_name $project->db_name -download_dir $project->project_dir/download -upload_url $project->upload_url -download_url $project->download_url -keyfile $project->key_dir/upload_private -appname $app->name -rsc_iops $this->rsc_iops -rsc_fpops $this->rsc_fpops -rsc_disk $this->rsc_disk -wu_template $this->wu_template -result_template $this->result_template -redundancy $this->redundancy -wu_name $this->wu_template -delay_bound $this->delay_bound";

        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            $cmd = $cmd." ".$x;
        }
        run_tool($cmd);
    }
}

function start_proxy($code)
{
    verbose_echo(1, "Starting proxy server");
    verbose_XPassThru("./testproxy 8080 localhost:80 '$code' 2>testproxy.log &");
}

function test_msg($msg)
{
    echo "-- Testing " . $msg . ' ' . str_repeat('-', 78 - strlen($msg)-12) . "\n";
}

function test_done()
{
    global $errors;

    // PHP's exit function is really annoying, it prints the exit value
    if ($errors) {
        verbose_echo(0, "ERRORS: $errors");

        exit ($errors);
    } else {
        verbose_echo(1, "Passed test!");
        if (VERBOSE == 1)
            echo "\n";
        exit();
    }
}

?>
