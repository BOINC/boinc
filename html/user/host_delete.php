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
    $host = lookup_host($hostid);
    if (!$host || $host->userid != $user->id) {
        fail("We have no record of that computer.");
    }
    return $host;
}

db_init();
$user = get_logged_in_user();

page_head("Delete record of computer");

$hostid = get_int("hostid");
$host = get_host($hostid, $user);
if (host_nresults($host)==0) {
    mysql_query("delete from host where id=$hostid");
} else {
    $config = get_config();
    $results = "existing results";
    if (parse_bool($config, "show_results")) {
        $nresults = host_nresults($host);
        if ($nresults) {
            $results = "<a href=results.php?hostid=$host->id>existing $nresults results</a>";
        }
    }
    fail("You can not delete this computer because the project database still contains work for it. ".
         "You must wait a few days until all the $results for this computer ".
         "have been deleted from the project database.");
}
echo "
    Host deleted.
    <p><a href=hosts_user.php>Return to list of your computers</a>
";
page_tail();

?>
