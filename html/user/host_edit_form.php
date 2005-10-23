<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");

db_init();
$user = get_logged_in_user();

$hostid = get_int("hostid");
$host = lookup_host($hostid);
if (!$host || $host->userid != $user->id) {
    error_page("We have no record of that computer");
}

page_head("Merge host");

$t = time_str($host->create_time);
echo "
    Sometimes BOINC assigns separate identities to the same computer by mistake.
    You can correct this by merging old identities with the newest one.
    <form name=blah action=host_edit_action.php>
    <input type=hidden name=id_0 value=$hostid>
    <p>
    Check the computers that are the same as $host->domain_name
    (created $t, computer ID $host->id):
    <p>
";

$result = mysql_query("select * from host where userid=$user->id");
$nhosts = 1;
start_table();
row_heading_array(array("", "name", "created", "computer ID"));
while ($host2 = mysql_fetch_object($result)) {
    if ($host->id == $host2->id) continue;
    if (!hosts_compatible($host, $host2)) continue;
    $t = time_str($host2->create_time);
    $x = $host2->domain_name;
    if ($x == "") {
        $x = "[no hostname]";
    }
    row_array(array(
        "<input type=checkbox name=id_$nhosts value=$host2->id>",
        $x,
        "$t",
        "$host2->id"
     ));
    $nhosts++;
    if ($nhosts==500) break;
}
end_table();
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
