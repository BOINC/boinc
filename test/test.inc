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

define("RESULT_STATE_DONE", 4);

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
    //echo "$cmd\n";
    PassThru($cmd);
}

function create_keys() {
    $key_dir = get_env_var("BOINC_KEY_DIR");
    $lib_dir = get_env_var("BOINC_SRC_DIR")."/lib";
    PassThru("$lib_dir/crypt_prog -genkey 1024 $key_dir/upload_private $key_dir/upload_public");
    PassThru("$lib_dir/crypt_prog -genkey 1024 $key_dir/code_sign_private $key_dir/code_sign_public");
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
    var $platform_name;

    function App_Version($app) {
        $this->app = $app;
        $this->version = 1;
        $this->exec_name = $app->name;
        $this->platform_name = get_env_var("BOINC_PLATFORM");
    }
}

class Project {
    var $name;
    var $users;
    var $apps;
    var $app_versions;
    var $plat_names;
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

    function Project() {
        $this->name = "test";
        $this->users = array();
        $this->apps = array();
        $this->app_versions = array();
        $this->plat_names = array();
        $this->db_passwd = "";
        $this->generate_keys = false;
        $this->shmem_key = get_env_var("BOINC_SHMEM_KEY");
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

    function add_platform($plat_name) {
        array_push($this->plat_names, $plat_name);
    }

    // Set up the database and directory structures for a project
    //
    function Install($scheduler_file = null) {
        $base_dir = get_env_var("BOINC_PROJECTS_DIR");
        $source_dir = get_env_var("BOINC_SRC_DIR");
        $cgi_url = get_env_var("BOINC_CGI_URL")."/".$this->name;
        $this->download_url = get_env_var("BOINC_HTML_URL")."/".$this->name."/download";
        //link download1...downloadn to download. Get this from reading the reading how many <urls>s in the template. For uploads 
        $this->upload_url = $cgi_url."/file_upload_handler";
        $this->scheduler_url = $cgi_url."/cgi";
        $this->project_dir = $base_dir."/".$this->name;
        $this->master_url = get_env_var("BOINC_HTML_URL")."/".$this->name."/html_user/index.php";
        PassThru("rm -rf $this->project_dir");
        PassThru("mkdir $this->project_dir");
            // make the CGI writeable in case scheduler writes req/reply files
        PassThru("mkdir $this->project_dir/cgi; chmod uog+w $this->project_dir/cgi");
        PassThru("mkdir $this->project_dir/upload; chmod uog+w $this->project_dir/upload");
        PassThru("mkdir $this->project_dir/download");
        PassThru("mkdir $this->project_dir/keys");
        PassThru("mkdir $this->project_dir/html_ops");
        PassThru("mkdir $this->project_dir/html_user");

        array_push($this->plat_names, get_env_var("BOINC_PLATFORM"));

        if ($this->generate_keys) {
            echo "KEY GENERATION NOT IMPLEMENTED YET\n";
        } else {
            $this->key_dir = get_env_var("BOINC_KEY_DIR");
            PassThru("cp $this->key_dir/* $this->project_dir/keys");
        }

        // set up the database
        //
        $this->user_name = get_env_var("BOINC_USER_NAME");
        if (!$this->db_name)
            $this->db_name = $this->user_name."_".$this->name;
        run_db_script("drop.sql", $this->db_name);
        run_db_script("schema.sql", $this->db_name);
        run_db_script("constraints.sql", $this->db_name);

        db_open($this->db_name);
        db_query("insert into project(name) values('$this->name')");

        for ($i=0; $i<sizeof($this->users); $i++) {
            $user = $this->users[$i];
            $now = time(0);
            db_query("insert into user values (0, $now, '$user->email_addr', '$user->name', 'foobar', '$user->authenticator', 'Peru', '12345', 0, 0, 0, '', '', 0)");
        }

        echo "adding apps\n";
        for ($i=0; $i<sizeof($this->apps); $i++) {
            $app = $this->apps[$i];
            $now = time(0);
            db_query("insert into app(name, create_time) values ('$app->name', $now)");
        }

        echo "adding platforms\n";
        for ($i=0; $i<sizeof($this->plat_names); $i++) {
            $platform = $this->plat_names[$i];
            run_tool("add platform -db_name $this->db_name -platform_name $platform");
        }

        echo "adding app versions\n";
        for ($i=0; $i<sizeof($this->app_versions); $i++) {
            $app_version = $this->app_versions[$i];
            $app = $app_version->app;
            if ($app->name == "core client") {
                $dir = "$source_dir/client";
                $exec_name = sprintf("boinc_%s.%s_%s", get_env_var("BOINC_MAJOR_VERSION"), get_env_var("BOINC_MINOR_VERSION"), $app_version->platform_name);
            } else {
                $dir = "$source_dir/apps";
                $exec_name = $app_version->exec_name;
            }
            run_tool("add app_version -db_name $this->db_name -app_name '$app->name' -platform_name $app_version->platform_name -version $app_version->version -download_dir $this->project_dir/download -download_url $this->download_url -code_sign_keyfile $this->key_dir/code_sign_private -exec_dir $dir -exec_files $exec_name");
        }
        
        // copy the user and administrative PHP files to the project dir,
        //
        PassThru("cp -f $source_dir/html_user/* $this->project_dir/html_user");
        PassThru("cp -f $source_dir/tools/country_select $this->project_dir/html_user");
        PassThru("cp -f $source_dir/html_ops/* $this->project_dir/html_ops");

        // copy the server programs to the project /cgi dir,
        // and make a config file there
        //

        // Copy the sched server in the cgi directory with the
        // cgi names given $source_dir/html_usr/schedulers.txt
        //
        if ($scheduler_file == null) {
            $scheduler_file = "schedulers.txt";        
            $f = fopen("$this->project_dir/html_user/schedulers.txt", "w");
            fputs($f,"<scheduler>".$this->scheduler_url."</scheduler>\n");
            fclose($f);
        } else {
            $f = fopen("$this->project_dir/html_user/$scheduler_file", "r");
            while(true) {
                $scheduler_url = fgets($f, 1000);
                if($scheduler_url == false) break;
                $temp = substr($scheduler_url,0,strrpos($scheduler_url,'<'));
                $cgi_name = substr($temp,strrpos($temp,'/')+1,strlen($temp) - strrpos($temp,'/'));
                PassThru("cp $source_dir/sched/cgi $this->project_dir/cgi/$cgi_name");
            }

            fclose($f);
        }

        PassThru("cp $source_dir/sched/cgi $this->project_dir/cgi/");
        // would have to be able to add more of these, copy several to cgi dir
        PassThru("cp $source_dir/sched/file_upload_handler $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/make_work $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/feeder $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/result_retry $this->project_dir/cgi/");
        PassThru("cp $source_dir/sched/validate_test $this->project_dir/cgi/");
        PassThru("cp $source_dir/html_ops/stripchart.cgi $this->project_dir/cgi/");
        $f = fopen("$this->project_dir/cgi/config.xml", "w");
        fputs($f, "<config>\n");
        fputs($f, "    <db_name>$this->db_name</db_name>\n");
        fputs($f, "    <db_passwd>$this->db_passwd</db_passwd>\n");
        fputs($f, "    <shmem_key>$this->shmem_key</shmem_key>\n");
        fputs($f, "    <key_dir>$this->key_dir</key_dir>\n");
        fputs($f, "    <download_url>$this->download_url</download_url>\n");
        fputs($f, "    <upload_url>$this->upload_url</upload_url>\n");
        fputs($f, "    <upload_dir>$this->project_dir/upload</upload_dir>\n");
        fputs($f, "    <user_name>$this->user_name</user_name>\n");
        fputs($f, "</config>\n");
        fclose($f);


        // put a file with the database name and other info in each directory
        //
        $f = fopen("$this->project_dir/html_user/config.xml", "w");
        fputs($f, "<db_name>$this->db_name</db_name>\n");
        fputs($f, "<db_passwd>$this->db_name</db_passwd>\n");
        fputs($f, "<download_url>$this->download_url</download_url>\n");
        fputs($f, "<cgi_url>$cgi_url</cgi_url>\n");
        fclose($f);
        PassThru("cp $this->project_dir/html_user/config.xml $this->project_dir/html_ops");

        // edit "index.php" in the user directory to have
        // the right file as the source for scheduler_urls, default is schedulers.txt
        $x = "sed -e s/FILE_NAME/$scheduler_file/ $this->project_dir/html_user/index.php > temp; mv temp $this->project_dir/html_user/index.php";
        echo "x is $x\n";
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
        $admin_url = $html_url."/".$this->name."/html_ops/index.html";
        echo "The admin URL for project $this->name is $admin_url\n";
    }
    
    // moves the masterindex file to temp after $time seconds (if not null).
    // This is used to test exponential backoff on the client side.
    //
    function delete_masterindex($time=null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("mv $this->project_dir/html_user/index.php $this->project_dir/html_user/temp");
    }
    
    // moves temp back to the masterindex after $time seconds(if not null).
    // This is used to test exponential backoff on the client side.
    //
    function reestablish_masterindex($time=null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("mv $this->project_dir/html_user/temp $this->project_dir/html_user/index.php");
          
    }
    
    // delete the cgi file for this project after $time if not null.
    // This is used to test exponential backoff on the client side.
    //
    function delete_scheduler($time=null,$cgi_num = null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("rm $this->project_dir/cgi/cgi$cgi_num");
    }
      
    // copies the cgi file back into the cgi directory.
    // This is used to test exponential backoff on the client side.
    //
    function reinstall_scheduler($time=null,$cgi_num=null) {
        $source_dir = get_env_var("BOINC_SRC_DIR");
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("cp $source_dir/sched/cgi $this->project_dir/cgi/cgi$cgi_num");
    }
    
    // moves the download directory to temp.
    // This is used to test exponential backoff on the client side.
    //
    function delete_downloaddir($time = null,$download_dir_num = null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("mv $this->project_dir/download$download_dir_num $this->project_dir/download_moved$download_dir_num");
         
    }
    
    // reinstalls the download directory.
    // This is used to test exponential backoff on the client side.
    //
    function reinstall_downloaddir($time = null ,$download_dir_num = null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("mv $this->project_dir/download_moved$download_dir_num $this->project_dir/download$download_dir_num");
    }
    
    function remove_file_upload_handler($time = null,$handler_num = null) {
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("rm $this->project_dir/cgi/file_upload_handler$handler_num");
    }
        
    function reinstall_file_upload_handler($time = null,$handler_num = null) {
        $source_dir = get_env_var("BOINC_SRC_DIR");
        if($time != null) {
            echo "\nsleeping for $time seconds";
            PassThru("sleep $time");
        }
        PassThru("cp $source_dir/sched/file_upload_handler $this->project_dir/cgi/file_upload_handler$handler_num");
    }

    // blocks until a file_upload_handler is running in the system,
    // kills it and returns
    //
    function kill_file_upload_handler() {
        while(true) {
            $pid = exec("pgrep -n file_up");
            if($pid != null) break;
        }
        PassThru("kill -9 $pid");
    }
          
    function start_feeder(){
        PassThru("cd $this->project_dir/cgi; ./feeder -asynch > feeder_out");
    }

    function start_make_work($work){
        $result_template_path = realpath($work->result_template);
        PassThru("cd $this->project_dir/cgi; make_work -asynch -result_template $result_template_path -wu_name $work->wu_template > make_work_out");
    }

    // run the validator asynchronously
    //
    function start_validate($app, $quorum) {
        PassThru("cd $this->project_dir/cgi; validate_test -asynch -app $app->name -quorum $quorum > validate_out");
    }

    // do one pass of validation
    //
    function validate($app, $quorum) {
        PassThru("cd $this->project_dir/cgi; validate_test -one_pass -app $app->name -quorum $quorum > validate_out");
    }

    // this should stop the feeder and any other daemons
    //
    function stop() {
        $f = fopen($this->project_dir."/cgi/stop_server", "w");
        fputs($f, "<quit/>\n");
        fclose($f);
    }

    function check_results($ntarget, $result) {
        $n = 0;
        db_open($this->db_name);
        $result = mysql_query("select * from result");
        while ($x = mysql_fetch_object($result)) {
            $n++;
            if ($result->state != null && $result->state != $x->state) {
                echo "ERROR: result $x->id: unexpected state $x->state\n";
            }
            if ($result->stderr_out != null) {
                if (substr($result->stderr_out, $x->stderr_out)==0) {
                    echo "ERROR: result $x->id: unexpected stderr_out $x->stderr_out\n";
                }
            }
            if ($result->exit_state != null && $result->exit_status != $x->exit_status) {
                echo "ERROR: result $x->id: unexpected exit_status $x->exit_status\n";
            }
        }
        if ($n != $ntarget) {
            echo "ERROR: expected $ntarget results, found $n.\n";
        }
    }

    function num_wus_left() {
        db_open($this->db_name);
        $result = mysql_query("select count(*) as cnt from result where state=2");
        $count = mysql_fetch_object($result);
        return $count->cnt;
    }

    function num_results_done() {
        db_open($this->db_name);
        $result = mysql_query("select count(*) as cnt from result where state=4");
        $count = mysql_fetch_object($result);
        return $count->cnt;
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
    
    function kill($boinc_pid,$time) {
        assert($boinc_pid != null);        
        if($time != null) {
            echo "\nsleepinf for $time seconds";
            PassThru("sleep $time");
        }

        PassThru("kill -9 $boinc_pid");
    }
 
    // forks a new process to run this host with the given $args.
    // the pid returned is the pid of the newly forked php process.
    // NOTE: to kill the newly started host,
    // get_new_boincpid() must be called and kill() must be called on it. 
    //
    function run_asynch($args) {
        $pid = pcntl_fork();
        if ($pid == -1) {
            return -1; 
        } else if ($pid) {
            return $pid; // we are the parent 
        } else { //we are the child
            echo "\nRunning core client asynch\n";
            $source_dir = get_env_var("BOINC_SRC_DIR");
            $platform = get_env_var("BOINC_PLATFORM");
            $exec_name = sprintf("boinc_%s.%s_%s", get_env_var("BOINC_MAJOR_VERSION"), get_env_var("BOINC_MINOR_VERSION"), $platform);
            PassThru("cd $this->host_dir; $source_dir/client/$exec_name $args > client.out");
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

    function run($args) {
          echo "\nRunning core client\n";
          $source_dir = get_env_var("BOINC_SRC_DIR");
          $platform = get_env_var("BOINC_PLATFORM");
          $exec_name = sprintf("boinc_%s.%s_%s", get_env_var("BOINC_MAJOR_VERSION"), get_env_var("BOINC_MINOR_VERSION"), $platform);
          PassThru("cd $this->host_dir; $source_dir/client/$exec_name $args > client.out");
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
        $enc_url = replace($project->master_url, "/", "_");
        $path= "$this->host_dir/projects/$enc_url/$filename";
        if (!file_exists($path)) {
            echo "ERROR: file $path doesn't exist\n";
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
    var $rsc_disk;
    var $delay_bound;
        
    function Work($app) {
        $this->app = $app;
        $this->input_files = array();
        $this->rcs_iops = 180000000000;
        $this->rcs_fpops = 100000000000;
        $this->rcs_disk = 1000000;
        $this->delay_bound = 1000;
        $this->redundancy = 1;
    }

    function install($project) {
        $app = $this->app;
        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            PassThru("cp $x $project->project_dir/download");
        }

if (false) {        // doesn't belong here; needs comment
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
                PassThru("ln -s $project->project_dir/download $project->project_dir/download$append");
            }
        }
        
        fclose($f);
        $source_dir = get_env_var("BOINC_SRC_DIR");
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
                 
                    PassThru("cp $source_dir/sched/file_upload_handler $project->project_dir/cgi/file_upload_handler$append");
                      
                }
            }
        }
        fclose($f);
                      
        $cmd = "create_work -db_name $project->db_name -download_dir $project->project_dir/download -upload_url $project->upload_url -download_url $project->download_url -keyfile $project->key_dir/upload_private -appname $app->name -rsc_iops $this->rcs_iops -rsc_fpops $this->rsc_fpops -rsc_disk $this->rsc_disk -wu_template $this->wu_template -result_template $this->result_template -redundancy $this->redundancy -wu_name $this->wu_template -delay_bound $this->delay_bound";

        for ($i=0; $i<sizeof($this->input_files); $i++) {
            $x = $this->input_files[$i];
            $cmd = $cmd." ".$x;
        }
        run_tool($cmd);
    }
}
}
