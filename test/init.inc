<?php

//define("HOME", "home/david");
//define("DOWNLOAD_DIR", "$HOME/html/download");
//define("UPLOAD_DIR", "$HOME/html/upload");
//define("PLATFORM_NAME", "i686-pc-linux-gnu");
define("VERSION", "1");
define("CORE_CLIENT", "boinc_".VERSION."_$BOINC_PLATFORM");
define("PROJECTS", "localhost.localdomain zoot");
    // directories to empty
//define("EMAIL_ADDR", "david@localhost.localdomain");
//define("URL_BASE", "http://localhost.localdomain/download");
    // data server URL root

include_once("../html_user/db.inc");

function clear_db() {
    PassThru("cd ../db; init_db");
}

$BOINC_DOWNLOAD_DIR = null;
$BOINC_UPLOAD_DIR = null;
$BOINC_PLATFORM = null;
$BOINC_EMAIL = null;
$BOINC_URL_BASE = null;

function check_env_vars() {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_UPLOAD_DIR;
    global $BOINC_PLATFORM;
    global $BOINC_EMAIL;
    global $BOINC_URL_BASE;

    $bad = false;
    $BOINC_DOWNLOAD_DIR = getenv("BOINC_DOWNLOAD_DIR");
    if ($BOINC_DOWNLOAD_DIR == null) {
        echo "Must define BOINC_DOWNLOAD_DIR\n";
        $bad = true;
    }
    $BOINC_UPLOAD_DIR = getenv("BOINC_UPLOAD_DIR");
    if ($BOINC_UPLOAD_DIR == null) {
        echo "Must define BOINC_UPLOAD_DIR\n";
        $bad = true;
    }
    $BOINC_PLATFORM = getenv("BOINC_PLATFORM");
    if ($BOINC_PLATFORM == null) {
        echo "Must define BOINC_PLATFORM\n";
        $bad = true;
    }
    $BOINC_EMAIL = getenv("BOINC_EMAIL");
    if ($BOINC_EMAIL == null) {
        echo "Must define BOINC_EMAIL\n";
        $bad = true;
    }
    $BOINC_URL_BASE = getenv("BOINC_URL_BASE");
    if ($BOINC_URL_BASE == null) {
        echo "Must define BOINC_URL_BASE\n";
        $bad = true;
    }
    if ($bad) exit();
}
function clear_data_dirs() {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_UPLOAD_DIR;

    $BOINC_DOWNLOAD_DIR = getenv("BOINC_DOWNLOAD_DIR");
    if ($BOINC_DOWNLOAD_DIR == null) {
        echo "Must define BOINC_DOWNLOAD_DIR\n";
        $bad = true;
    }
    $BOINC_UPLOAD_DIR = getenv("BOINC_UPLOAD_DIR");
    if ($BOINC_UPLOAD_DIR == null) {
        echo "Must define BOINC_UPLOAD_DIR\n";
        $bad = true;
    }
    if ($bad) exit();
 
    PassThru("rm -f $BOINC_DOWNLOAD_DIR/*");
    PassThru("rm -f $BOINC_UPLOAD_DIR/*");
}

function init_client_dirs($prefs_file) {
    PassThru("rm -f client_state.xml");
    PassThru("rm -rf ".PROJECTS);
    PassThru("rm -rf slots");
    PassThru("cp $prefs_file prefs.xml");
}

function copy_to_download_dir($f) {
    global $BOINC_DOWNLOAD_DIR;

    PassThru("cp $f $BOINC_DOWNLOAD_DIR");
}

function add_user($prefs_file) {
    global $BOINC_EMAIL;

    $cmd = "../tools/add user -email_addr $BOINC_EMAIL -user_name David -web_password foobar -authenticator 3f7b90793a0175ad0bda68684e8bd136 ";
    if ($prefs_file) {
        $cmd = $cmd." -prefs_file $prefs_file";
    }
    PassThru($cmd);
}

function add_prefs($file) {
    global $BOINC_EMAIL;

    PassThru("../tools/add prefs -email_addr $BOINC_EMAIL -prefs_file $file");
}

function add_platform($platform) {
    global $BOINC_PLATFORM;

    if( $platform ) {
        PassThru("../tools/add platform -platform_name $platform");
    } else {
        PassThru("../tools/add platform -platform_name $BOINC_PLATFORM");
    }
}

function add_core_client_message($message, $priority) {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_UPLOAD_DIR;
    global $BOINC_PLATFORM;

    PassThru("../tools/add app -app_name core_client -version ".VERSION);
    PassThru("../tools/add app_version -app_name core_client -platform_name $BOINC_PLATFORM -version ".VERSION." -exec_dir ../client -exec_file ".CORE_CLIENT." -download_dir $BOINC_DOWNLOAD_DIR -url_base $BOINC_URL_BASE -message '$message' -message_priority '$priority'");
    PassThru("cp ../client/".CORE_CLIENT." $BOINC_DOWNLOAD_DIR");
}

function add_core_client() {
    add_core_client_message("", "");
}

function add_app($name) {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_PLATFORM;
    global $BOINC_URL_BASE;

    PassThru("../tools/add app -app_name $name -version ".VERSION);
    PassThru("../tools/add app_version -app_name $name -platform_name $BOINC_PLATFORM -version ".VERSION." -exec_dir ../apps -exec_file $name -download_dir $BOINC_DOWNLOAD_DIR -url_base $BOINC_URL_BASE");
    PassThru("cp ../apps/$name $BOINC_DOWNLOAD_DIR");
}

function create_work($x) {
    PassThru("../tools/create_work $x");
}

function run_client() {
    PassThru("../client/".CORE_CLIENT." -exit_when_idle");
}

function start_feeder() {
    PassThru("cd ../sched; feeder -asynch > feeder_out");
}

function stop_feeder() {
    $f = fopen("../sched/feeder_trigger", "w");
    fputs($f, "<quit/>\n");
    fclose($f);
}

function compare_file($out, $correct) {
    global $BOINC_UPLOAD_DIR;

    PassThru("diff $BOINC_UPLOAD_DIR/$out $correct", $retval);
    if ($retval) {
        echo "File mismatch: $out $correct\n";
    } else {
        echo "Files match: $out $correct\n";
    }
}

function check_results_done() {
    db_init();
    $result = mysql_query("select * from result where state<>4");
    while ($x = mysql_fetch_object($result)) {
        echo "result $x->id is not done\n";
    }
}

function compare_files($out, $correct) {
    PassThru("diff $out $correct", $retval);
    if ($retval) {
        echo "File mismatch: $out $correct\n";
    } else {
        echo "Files match: $out $correct\n";
    }
}

function initialize_api() {
    PassThru("cp core_to_app.xml.in core_to_app.xml");
}

function run_api_test() {
    PassThru("../api/api_test");
}

?>
