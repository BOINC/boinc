<?php

require_once("db.inc");
require_once("util.inc");
require_once("host.inc");

db_init();
$user = get_logged_in_user();

page_head("Merge host", $user);

$hostid = $_GET["hostid"];
$result = mysql_query("select * from host where id=$hostid");
$host = mysql_fetch_object($result);
mysql_free_result($result);
if (!$host || $host->userid != $user->id) {
    echo "Host not found";
    exit();
}

$t = time_str($host->create_time);
echo "
    Sometimes BOINC assigns separate identities to the same host.
    You can correct this by merging old identities with the newest one.
    <form action=host_edit_action.php>
    <input type=hidden name=hostid value=$hostid>
    <p>
    Merge $host->domain_name (created $t) with
    <select name=targetid>
    <option value=0> -
";

$result = mysql_query("select * from host where userid=$user->id");
while ($host2 = mysql_fetch_object($result)) {
    if ($host->id == $host2->id) continue;
    if ($host2->create_time < $host->create_time) continue;
    if (!hosts_compatible($host, $host2)) continue;
    $t = time_str($host2->create_time);
    echo "<option value=$host2->id> $host2->domain_name (created $t)\n";
}
echo "
    </select>
    <input type=submit value='Merge host'>
    </form>
";

page_tail();

?>
