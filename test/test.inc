<?php

// test.inc
//
// A set of classes for testing BOINC.
// These classes let you create multiple projects and multiple hosts
// (all running on a single machine),
// add applications and work units, run the system,
// and verify that the results are correct.
//
// See doc/test.html for details

// get an enviroment variable, and abort script if missing
//
function get_env_var($name) {
    $x = getenv($name);
    if ($x == null) {
        echo "Environment variable $name not defined\n";
        exit();
    }
    return $x;
}

function run_db_script($script, $db_name) {
    $db_dir = get_env_var("BOINC_SRC_DIR")."/db";
    $x = "sed -e s/BOINC_DB_NAME/$db_name/ $db_dir/$script | mysql";
    //echo $x;
    PassThru($x);
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
    $tool_dir = get_env_var("BOINC_SRC_DIR")."/tools/";
    $cmd = $tool_dir.$cmd;
    PassThru($cmd);
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
    var $exec_name;

    function App_Version($app) {
        $this->app = $app;
        $this->version = 1;
        $this->exec_name = $app->name;
    }
}

class Project {
    var $name;
    var $users;
    var $apps;
    var $app_versions;
    var $project_dir;
    var $db_name;
    var $generate_keys;
    var $shmem_key;
    var $key_dir;
    var $download_url;
    var $scheduler_url;
    var $upload_url;
    var $user_name;
    var $master_url;
    var $resource_share;

    function Project() {
        $this->name = "test";
        $this->users = array();
        $this->apps = array();
        $this->app_versions = array();
        $this->generate_keys = false;
        $this->shmem_key = "0xbeefacafe";
        $this->resource_share = 1;
    }

    function add_user($user) {
        array_push($this->users, $user);
    }

    function add_app($app) {
        array_push($this->apps, $app);
    }

    function add_app_version($app_version) {
        array_push($this->app_versions, $app_version);
    }

    // Set up the database and directory structures for a project
    //
    function Install() {
        $base_dir = get_env_var("BOINC_PROJECTS_DIR");
        $source_dir = get_env_var("BOINC_SRC_DIR");
        $this->download_url = get_env_var("BOINC_HTML_URL")."/".$this->name."/download";
        $this->upload_url = get_env_var("BOINC_CGI_URL")."/".$this->name."/file_upload_handler";
        $this->scheduler_url = get_env_var("BOINC_CGI_URL")."/".$this->name."/cgi";
        $this->project_dir = $base_dir."/".$this->name;
        $this->master_url = get_env_var("BOINC_HTML_URL")."/".$this->name."/html_user/index.html";
        PassThru("rm -rf $this->project_dir");
        PassThru("mkdir $this->project_dir");
        PassThru("mkdir $this->project_dir/cgi");
        PassThru("mkdir $this->project_dir/upload; chmod uog+w $this->project_dir/upload");
        PassThru("mkdir $this->project_dir/download");
        PassThru("mkdir $this->project_dir/keys");
        PassThru("mkdir $this->project_dir/html_ops");
        PassThru("mkdir $this->project_dir/html_user");

        if ($this->generate_keys) {
        } else {
            $this->key_dir = get_env_var("BOINC_KEY_DIR");
            PassThru("cp $this->key_dir/* $this->project_dir/keys");
        }

        // set up the database
        //
        $this->user_name = get_env_var("BOINC_USER_NAME");
        $this->db_name = $this->user_name."_".$this->name;
        run_db_script("drop.sql", $this->db_name);
        run_db_script("schema.sql", $this->db_name);
        run_db_script("constraints.sql", $this->db_name);

        db_open($this->db_name);
        db_query("insert into project(name) values('$this->name')");

        $platform = get_env_var("BOINC_PLATFORM");
        db_query("insert into platform(name) values('$platform')");

        for ($i=0; $i<sizeof($this->users); $i++) {
            $user = $this->users[$i];
            $now = time(0);
            db_query("insert into user values (0, $now, '$user->email_addr', '$user->name', 'foobar', '$user->authenticator', 'Peru', '12345', 0, 0, 0, '', '', 0)");
        }

        echo "adding apps\n";
        for ($i=0; $i<sizeof($this->apps); $i++) {
            $app = $this->apps[$i];
            db_query("insert into app(name, create_time) values ('$app->name', $now)");
        }

        echo "adding app versions\n";
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app_version = $this->app_versions[$i];
            $app = $app_version->app;
            run_tool("add app_version -db_name $this->db_name -app_name $app->name -platform_name $platform -version $app_version->version -download_dir $this->project_dir/download -download_url $this->download_url -code_sign_keyfile $this->key_dir/code_sign_private -exec_dir $source_dir/apps -exec_files $app_version->exec_name");
        }

        // copy the CGI programs and feeder to the project dir,
        // and make a config file there
        //
        PassThru("cp $source_dir/sched/cgi $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/file_upload_handler $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/feeder $this->project_dir/cgi/");
        $f = fopen("$this->project_dir/cgi/config.xml", "w");
        fputs($f, "<config>\n");
        fputs($f, "    <db_name>$this->db_name</db_name>\n");
        fputs($f, "    <db_passwd>$this->db_passwd</db_passwd>\n");
        fputs($f, "    <shmem_key>$this->shmem_key</shmem_key>\n");
        fputs($f, "    <key_dir>$this->key_dir</key_dir>\n");
        fputs($f, "    <upload_dir>$this->project_dir/upload</upload_dir>\n");
        fputs($f, "    <user_name>$this->user_name</user_name>\n");
        fputs($f, "</config>\n");
        fclose($f);

        // copy the user and administrative PHP files to the project dir,
        //
        PassThru("cp -f $source_dir/html_user/* $this->project_dir/html_user");
        PassThru("cp -f $source_dir/html_ops/* $this->project_dir/html_ops");

        // put a file with the database name in each directory
        //
        $f = fopen("$this->project_dir/html_user/db_name", "w");
        fputs($f, "$this->db_name\n");
        fclose($f);
        $f = fopen("$this->project_dir/html_ops/db_name", "w");
        fputs($f, "$this->db_name\n");
        fclose($f);

        // edit "index.html" in the user directory to have
        // the right scheduler URL
        //
        $u = str_replace("/", "\\\/", $this->scheduler_url);
        $x = "sed -e s/SCHEDULER_URL/$u/ $this->project_dir/html_user/index.html > temp; mv temp $this->project_dir/html_user/index.html";
        echo "$x\n";
        PassThru($x);

        // create symbolic links to the CGI and HTML directories
        //
        $cgi_dir = get_env_var("BOINC_CGI_DIR");
        $cgi_url = get_env_var("BOINC_CGI_URL");
        $html_dir = get_env_var("BOINC_HTML_DIR");
        $html_url = get_env_var("BOINC_HTML_URL");
        PassThru("rm -f $cgi_dir/$this->name");
        PassThru("ln -s $this->project_dir/cgi $cgi_dir/$this->name");
        PassThru("rm -f $html_dir/$this->name");
        PassThru("ln -s $this->project_dir $html_dir/$this->name");

        // show the URLs for user and admin sites
        //
        echo "The master URL for project $this->name is $this->master_url\n";
        $admin_url = $html_url.$this->name."/html_ops/index.html";
        echo "The admin URL for project $this->name is $admin_url\n";
    }

    function start(){
        PassThru("cd $this->project_dir/cgi; feeder -asynch > feeder_out");
    }

    function stop() {
        $f = fopen($this->project_dir."/cgi/feeder_trigger", "w");
        fputs($f, "<quit/>\n");
        fclose($f);
    }

    function check_results_done() {
        db_open($this->db_name);
        $result = mysql_query("select * from result where state<>4");
        while ($x = mysql_fetch_object($result)) {
            echo "result $x->id is not done\n";
        }
    }

    function num_results_done() {
        db_open($this->db_name);
        $result = mysql_query("select count(*) from result where state=4");
        return mysql_fetch_object($result);
    }

    function compare_file($result, $correct) {
        PassThru("diff $this->project_dir/upload/$result $correct", $retval);
        if ($retval) {
            echo "File mismatch: $out $correct\n";
        } else {
            echo "Files match: $out $correct\n";
        }
    }
}


class User {
    var $name;
    var $email_addr;
    var $authenticator;

    function User() {
        $this->name = "John";
        $this->email_addr = "john@boinc.org";
        $this->authenticator = "3f7b90793a0175ad0bda68684e8bd136";
    }
}


class Host {
    var $name;
    var $projects;
    var $user;
    var $host_dir;
    var $global_prefs;
    var $log_flags;

    function Host($user) {
        $this->user = $user;
        $this->name = "test";
        $this->projects = array();
        $this->global_prefs = null;
        $this->log_flags = null;
    }

    function add_project($project) {
        array_push($this->projects, $project);
    }

    function install() {
        $base_dir = get_env_var("BOINC_HOSTS_DIR");
        $this->host_dir = $base_dir."/".$this->name;
        $user = $this->user;
        PassThru("rm -rf $this->host_dir");
        PassThru("mkdir $this->host_dir");

        // create account files
        //
        echo "creating account files\n";
        for ($i=0; $i<sizeof($this->projects); $i++) {
            $project = $this->projects[$i];
            $encoded_name = strtr($project->name, "/", "_");
            echo "writing $this->host_dir/account_$encoded_name.xml\n";
            $f = fopen($this->host_dir."/account_$encoded_name.xml", "w");
            fputs($f, "<account>\n");
            fputs($f, "    <master_url>$project->master_url</master_url>\n");
            fputs($f, "    <authenticator>$user->authenticator</authenticator>\n");
            fputs($f, "    <resource_share>$project->resource_share</resource_share>\n");
            fputs($f, "</account>\n");
            fclose($f);
        }

        // copy log flags, if any
        //
        if ($this->log_flags != null) {
            PassThru("cp $this->log_flags $this->host_dir/log_flags.xml");
        }

        // copy global prefs, if any
        //
        if ($this->global_prefs != null) {
            PassThru("cp $this->global_prefs.xml $this->host_dir/global_prefs.xml");
        }
    }

    function run($args) {
        echo "Running core client\n";
        $source_dir = get_env_var("BOINC_SRC_DIR");
        $platform = get_env_var("BOINC_PLATFORM");
        PassThru("cd $this->host_dir; $source_dir/client/boinc_1_$platform $args");
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

}

class Work {
    var $app;
    var $wu_template;
    var $result_template;
    var $nresults;
    var $input_files;
    var $rsc_iops;
    var $rsc_fpops;
    var $rsc_disk;

    function Work($app) {
        $this->app = $app;
        $this->input_files = array();
        $this->rcs_iops = 180000000000;
        $this->rcs_fpops = 100000000000;
        $this->rcs_disk = 1000000;
    }

    function install($project) {
        $app = $this->app;
        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            PassThru("cp $x $project->project_dir/download");
        }
        $cmd = "create_work -db_name $project->db_name -download_dir $project->project_dir/download -upload_url $project->upload_url -download_url $project->download_url/ -keyfile $project->key_dir/upload_private -appname $app->name -rsc_iops $this->rcs_iops -rsc_fpops $this->rsc_fpops -rsc_disk $this->rsc_disk -wu_name $this->wu_template -wu_template $this->wu_template -result_template $this->result_template -nresults $this->nresults ";
        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            $cmd = $cmd." ".$x;
        }
        run_tool($cmd);
    }
}
