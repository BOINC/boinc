<?php

require_once("db.inc");
require_once("util.inc");

function show_profile($profile, $n) {
    $user = lookup_user_id($profile->userid);
    echo "<br>". user_links($user)."\n";
}

db_init();

$search_string = $_GET['search_string'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

page_head("Profile search results");

echo "<h2>Profiles containing '$search_string'</h2>\n";
$q = "select * from profile where match(response1, response2) against ('$search_string') limit $offset,$count";
$result = mysql_query($q);
echo "<table>";
$n = 0;
while ($profile = mysql_fetch_object($result)) {
    show_profile($profile, $n+$offset+1);
    $n += 1;
}
echo "</table>";
mysql_free_result($result);
if ($offset==0 && $n==0) {
    echo "No profiles found containing '$search_string'";
}

if ($n==$count) {
    $s = urlencode($search_string);
    $offset += $count;
    echo "
        <a href=profile_search_action.php?search_string=$s&offset=$offset>Next $count</a>
    ";

}

page_tail();
?>
