<?php

// Handler for TreeThreader remote job submission.
//
// Assumptions:
// - there is a file "three_threader_templates" in the project root
//   containing (one per line) the names of files containing
//   gzipped collections of template files
// - These files are in the download hierarchy.

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/xml.inc");
require_once("../inc/dir_hier.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

function error($s) {
    echo "<error>\n<message>$s</message>\n</error>\n";
    exit;
}

function handle_submit($r, $user) {
    
    $batch_id = BoincBatch::insert("(user_id, create_time, njobs, name, app_id, state) values ($user->id, $now, $njobs, '$batch_name', $app->id, ".BATCH_STATE_IN_PROGRESS.")"
    );
}

function handle_abort($r) {
}

function handle_status($r) {
}

function handle_get_output($r) {
}

function handle_retire($r) {
}

xml_header();

$r = simplexml_load_string($_POST['request']);

if (!$r) {
    error("can't parse request message");
}

// authenticate the user
//
$auth = (string)$r->auth;
$user = BoincUser::lookup("auth='$auth'");
if (!$user) error("invalid authenticator");
$user_submit = BoincUserSubmit::lookup_userid($user->id);
if (!$user_submit) error("no submit access");
$app = BoincApp::lookup("name='tree_threader'");
if (!$app) error("no tree_threader app");

if (!$user_submit->submit_all) {
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
    if (!$usa) {
        error("no submit access");
    }
}

switch ($r->getName()) {
    case 'submit': handle_submit($r, $user); break;
    case 'abort': handle_abort($r); break;
    case 'status': handle_status($r); break;
    case 'get_output': handle_get_output($r); break;
    case 'retire': handle_retire($r); break;
    default: error("bad command");
}

?>
