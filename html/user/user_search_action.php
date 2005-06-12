<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function show_user($user, $n) {
    echo "<br>". user_links($user)."\n";
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
echo "<table>";
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
