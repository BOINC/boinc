<?php
require_once("docutil.php");
require_once("poll.inc");
require_once("../html/inc/translation.inc");
require_once("poll_data.inc");

boinc_error_page("The poll is closed");
function error() {
    page_head(tra("Error - results not recorded"));
    echo tra("An internal error has prevented us from recording your survey response.  Please try again later.");
    page_tail();
    exit();
}

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("poll");

session_set_cookie_params(86400*365);
session_start();
$uid = session_id();

$x = parse_form();
$xml = generate_xml($x);

$response = select_response($uid);
if ($response) {
    $result = update_response($uid, $xml);
} else {
    $result = new_response($uid, $xml);
}
if ($result) {
    page_head(tra("Survey response recorded"));
    echo tra("Thank you for completing the BOINC user survey.");
    page_tail();
} else {
    error();
}

?>
