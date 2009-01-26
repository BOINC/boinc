<?php
require_once("docutil.php");
require_once("poll.inc");
require_once("../html/inc/translation.inc");
require_once("poll_data.inc");

mysql_pconnect("localhost", "boincadm", null);
mysql_select_db("poll");

session_set_cookie_params(86400*365);
session_start();
$uid = session_id();

$response = select_response($uid);
if ($response) {
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
    echo "<response>
        $response->xml
        </response>
    ";
} else {
    echo "no response";
}

?>
