<?php

// Handler for TreeThreader remote job submission.

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

function handle_submit($r) {
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

switch ($r->getName()) {
    case 'submit': handle_submit($r); break;
    case 'abort': handle_abort($r); break;
    case 'status': handle_status($r); break;
    case 'get_output': handle_get_output($r); break;
    case 'retire': handle_retire($r); break;
    default: error("bad command");
}

?>
