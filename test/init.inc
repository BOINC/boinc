<?php

define("VERSION", "1");
define("CORE_CLIENT", "boinc_".VERSION."_$BOINC_PLATFORM");
define("PROJECTS", "localhost.localdomain zoot");

include_once("../html_user/db.inc");

function clear_db() {
    PassThru("cd ../db; init_db");
}

$BOINC_DOWNLOAD_DIR = null;
$BOINC_UPLOAD_DIR = null;
$BOINC_PLATFORM = null;
$BOINC_EMAIL = null;
$BOINC_URL_BASE = null;
$BOINC_KEY_DIR = null;
$BOINC_MASTER_URL = null;

function check_env_vars() {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_UPLOAD_DIR;
    global $BOINC_PLATFORM;
    global $BOINC_EMAIL;
    global $BOINC_URL_BASE;
    global $BOINC_KEY_DIR;
    global $BOINC_MASTER_URL;

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
    $BOINC_KEY_DIR = getenv("BOINC_KEY_DIR");
    if ($BOINC_KEY_DIR == null) {
        echo "Must define BOINC_KEY_DIR\n";
        $bad = true;
    }
    $BOINC_MASTER_URL = getenv("BOINC_MASTER_URL");
    if ($BOINC_MASTER_URL == null) {
        echo "Must define BOINC_MASTER_URL\n";
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
    $BOINC_KEY_DIR = getenv("BOINC_KEY_DIR");
    if ($BOINC_KEY_DIR == null) {
        echo "Must define BOINC_KEY_DIR\n";
        $bad = true;
    }
    if ($bad) exit();
 
    PassThru("rm -f $BOINC_DOWNLOAD_DIR/*");
    PassThru("rm -f $BOINC_UPLOAD_DIR/*");
    PassThru("rm -f $BOINC_KEY_DIR/*");
}

function init_client_dirs($prefs_file) {
    global $BOINC_MASTER_URL;

    PassThru("rm -f client_state.xml");
    PassThru("rm -rf ".PROJECTS);
    PassThru("rm -rf slots");
    PassThru("sed -e s/BOINC_MASTER_URL/\$BOINC_MASTER_URL/ $prefs_file > prefs.xml");
}

function copy_to_download_dir($f) {
    global $BOINC_DOWNLOAD_DIR;

    PassThru("cp $f $BOINC_DOWNLOAD_DIR");
}

function add_user($prefs_file) {
    global $BOINC_EMAIL;
    PassThru("sed -e s/BOINC_MASTER_URL/\$BOINC_MASTER_URL/ $prefs_file > prefs_temp.xml");
    $cmd = "../tools/add user -email_addr $BOINC_EMAIL -user_name David -web_password foobar -authenticator 3f7b90793a0175ad0bda68684e8bd136 ";
    if ($prefs_file) {
        $cmd = $cmd." -prefs_file prefs_temp.xml";
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

function add_core_client_message($message, $priority, $platform) {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_UPLOAD_DIR;
    global $BOINC_PLATFORM;
    global $BOINC_KEY_DIR;

    if( $platform == null ) {
        $plat = $BOINC_PLATFORM;
    } else {
        $plat = $platform;
    }
    PassThru("../tools/add app -app_name core_client -version ".VERSION);
    PassThru("../tools/add app_version -app_name core_client -platform_name $plat -version ".VERSION." -exec_dir ../client -exec_file ".CORE_CLIENT." -download_dir $BOINC_DOWNLOAD_DIR -url_base $BOINC_URL_BASE -message '$message' -message_priority '$priority' -code_sign_keyfile $BOINC_KEY_DIR/code_sign_private");
    PassThru("cp ../client/".CORE_CLIENT." $BOINC_DOWNLOAD_DIR");
}

function add_core_client($platform) {
    add_core_client_message("", "", $platform);
}

function add_app($name, $platform, $exec_name) {
    PassThru("../tools/add app -app_name $name -version ".VERSION);
    add_app_version( $name, $platform, $exec_name );
}

function add_app_version($name, $platform, $exec_name) {
    global $BOINC_DOWNLOAD_DIR;
    global $BOINC_PLATFORM;
    global $BOINC_URL_BASE;
    global $BOINC_KEY_DIR;

    if( $exec_name == null ) {
        $exec_name = $name;
    }
    if( $platform == null ) {
        $plat = $BOINC_PLATFORM;
    } else {
        $plat = $platform;
    }

    PassThru("../tools/add app_version -app_name $name -platform_name $plat -version ".VERSION." -exec_dir ../apps -exec_file $exec_name -download_dir $BOINC_DOWNLOAD_DIR -url_base $BOINC_URL_BASE -code_sign_keyfile $BOINC_KEY_DIR/code_sign_private");
    PassThru("cp ../apps/$exec_name $BOINC_DOWNLOAD_DIR");
}

function create_work($x) {
    global $BOINC_KEY_DIR;
    PassThru("../tools/create_work -keyfile $BOINC_KEY_DIR/upload_private $x");
}

function create_keys() {
    global $BOINC_KEY_DIR;
    PassThru("../lib/crypt_prog -genkey 1024 $BOINC_KEY_DIR/upload_private $BOINC_KEY_DIR/upload_public");
    PassThru("../lib/crypt_prog -genkey 1024 $BOINC_KEY_DIR/code_sign_private $BOINC_KEY_DIR/code_sign_public");
}

function run_client($args) {
    PassThru("../client/".CORE_CLIENT." $args");
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

function num_wus_left() {
    db_init();
    $numwus = mysql_query("select count(*) as nres from result where state<>4");
    $result = mysql_fetch_object($numwus);
    return $result->nres;
}

function compare_files($out, $correct) {
    PassThru("diff $out $correct", $retval);
    if ($retval) {
        echo "File mismatch: $out $correct\n";
    } else {
        echo "Files match: $out $correct\n";
    }
}

function run_api_test() {
    PassThru("../api/api_test");
}

function clean_api() {
    PassThru("rm -f counter app_to_core.xml core_to_app.xml foobar");
}

function compare_time() {
    $epsilon = 0.0001;
    db_init();
    $data = mysql_query("select cpu_time from result where name = 'uccpu_wu_0'");
    $result = mysql_fetch_object($data);
    $db_time = $result->cpu_time;
    $time_file = fopen("time.xml", "r");
    fscanf($time_file, "%f", $app_time);
    if(abs($db_time - $app_time) > $epsilon) {
        printf("Time mismatch: app %f server %f\n", $app_time, $db_time);
    }
    else printf("Times match\n");
    fclose($time_file);
    PassThru("rm -f time.xml");
}

?>
