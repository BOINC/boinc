<?php

define("HOME", "home/david");
define("DOWNLOAD_DIR", "$HOME/html/download");
define("UPLOAD_DIR", "$HOME/html/upload");
define("PLATFORM_NAME", "i686-pc-linux-gnu");
define("VERSION", "1");
define("CORE_CLIENT", "boinc_".VERSION."_".PLATFORM_NAME);
define("PROJECTS", "localhost.localdomain zoot");
    // directories to empty
define("EMAIL_ADDR", "david@localhost.localdomain");
define("URL_BASE", "http://localhost.localdomain/download");
    // data server URL root

include_once("../html_user/db.inc");

function clear_db() {
    PassThru("cd ../db; init_db");
}

function clear_data_dirs() {
    PassThru("rm -f ".DOWNLOAD_DIR."/*");
    PassThru("rm -f ".UPLOAD_DIR."/*");
}

function init_client_dirs($account_file) {
    PassThru("rm -f client_state.xml");
    PassThru("rm -rf ".PROJECTS);
    PassThru("rm -rf slots");
    PassThru("cp $account_file account.xml");
}

function copy_to_download_dir($f) {
    PassThru("cp $f ".DOWNLOAD_DIR);
}

function add_user() {
    PassThru("../tools/add user -email_addr ".EMAIL_ADDR." -user_name David -web_password foobar -authenticator 3f7b90793a0175ad0bda68684e8bd136");
}

function add_prefs($file) {
    PassThru("../tools/add prefs -email_addr ".EMAIL_ADDR." -prefs_file $file");
}

function add_platform() {
    PassThru("../tools/add platform -platform_name ".PLATFORM_NAME);
}

function add_core_client_message($message, $priority) {
    PassThru("../tools/add app -app_name core_client -version ".VERSION);
    PassThru("../tools/add app_version -app_name core_client -platform_name ".PLATFORM_NAME." -version ".VERSION." -exec_dir ../client -exec_file ".CORE_CLIENT." -download_dir ".DOWNLOAD_DIR." -url_base ".URL_BASE." -message '$message' -message_priority '$priority'");
    PassThru("cp ../client/".CORE_CLIENT." ".DOWNLOAD_DIR);
}

function add_core_client() {
    add_core_client_message("", "");
}

function add_app($name) {
    PassThru("../tools/add app -app_name $name -version ".VERSION);
    PassThru("../tools/add app_version -app_name $name -platform_name ".PLATFORM_NAME." -version ".VERSION." -exec_dir ../apps -exec_file $name -download_dir ".DOWNLOAD_DIR." -url_base ".URL_BASE);
    PassThru("cp ../apps/$name ".DOWNLOAD_DIR);
}

function create_work($x) {
    PassThru("../tools/create_work $x");
}

function run_client() {
    PassThru("../client/".CORE_CLIENT." -exit_when_idle");
}

function compare_file($out, $correct) {
    PassThru("diff ".UPLOAD_DIR."/$out $correct", $retval);
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


?>
