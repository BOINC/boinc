<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function show_user($user, $n) {
    echo "<tr><td align=\"center\">".user_links($user)."</td><td align=\"center\">".date_str($user->create_time)."</td><td align=\"center\">".$user->country."</td><td align=\"center\">".(int)$user->total_credit."</td><td align=\"center\">".(int)$user->expavg_credit."</td></tr>\n";
}

db_init();

$search_string = $_GET['search_string'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

page_head("Search results");

echo "<h2>User names starting with '$search_string'</h2>\n";

$search_string = str_replace('_', '\\\\_', $search_string);
$search_string = str_replace('%', '\\\\%', $search_string);

$q = "select * from user where name like '$search_string%' limit $offset,$count";
$result = mysql_query($q);
echo "<table align=\"center\" cellpadding=\"2\" border=\"1\" width=\"90%\"><tr><th align=\"center\">User name</th><th align=\"center\">Joined project</th><th align=\"center\">Country</th><th align=\"center\">Total credit</th><th align=\"center\">Recent credit</th></tr>";
$n = 0;
while ($user = mysql_fetch_object($result)) {
    show_user($user, $n+$offset+1);
    $n += 1;
}
echo "</table>";
mysql_free_result($result);
if ($offset==0 && $n==0) {
    echo "No user names found starting with '$search_string'";
}

if ($n==$count) {
    $s = urlencode($search_string);
    $offset += $count;
    echo "
        <br><br>
        <a href=user_search_action.php?search_string=$s&offset=$offset>Next $count</a>
    ";

}

page_tail();
?>
