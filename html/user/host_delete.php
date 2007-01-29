<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");

function fail($msg) {
    echo "<h2>Error: $msg</h2>";
    page_tail();
    exit();
}

function get_host($hostid, $user) {
    return $host;
}

db_init();
$user = get_logged_in_user();

$hostid = get_int("hostid");
$host = lookup_host($hostid);
if (!$host || $host->userid != $user->id) {
    error_page("We have no record of that computer.");
}

$nresults = host_nresults($host);
if ($nresults == 0) {
    mysql_query("delete from host where id=$hostid");
} else {
    error_page(
        "You can not delete our record of this computer because our
        database still contains work for it.
        You must wait a few days until the work for this computer
        have been deleted from the project database."
    );
}
page_head("Delete record of computer");
echo "
    Record deleted.
    <p><a href=hosts_user.php>Return to list of your computers</a>
";
page_tail();

?>
