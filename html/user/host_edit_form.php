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
    Sometimes BOINC assigns separate identities to the same computer by mistake.
    You can correct this by merging old identities with the newest one.
    <form name=blah action=host_edit_action.php>
    <input type=hidden name=id_0 value=$hostid>
    <p>
    Check the computers that are the same as $host->domain_name (created $t):
    <p>
";

$result = mysql_query("select * from host where userid=$user->id");
$nhosts = 1;
while ($host2 = mysql_fetch_object($result)) {
    if ($host->id == $host2->id) continue;
    //if ($host2->create_time > $host->create_time) continue;
    if (!hosts_compatible($host, $host2)) continue;
    $t = time_str($host2->create_time);
    echo "<br><input type=checkbox name=id_$nhosts value=$host2->id> $host2->domain_name (created $t)\n";
    $nhosts++;
    if ($nhosts==500) break;
}
mysql_free_result($result);
echo "
    <br>
    <script>
        function set_all() {
";
for ($i=1; $i<$nhosts; $i++) {
    echo "document.blah.id_$i.checked=1;\n";
}
echo "
        }
        function clear_all() {
";
for ($i=1; $i<$nhosts; $i++) {
    echo "document.blah.id_$i.checked=0;\n";
}
echo "
        }
    </script>
    <p><a href=javascript:set_all()>Select all</a>
    <p><a href=javascript:clear_all()>Unselect all</a>
    <input type=hidden name=nhosts value=$nhosts>
    <p><input type=submit value='Merge hosts'>
    </form>
";

page_tail();

?>
