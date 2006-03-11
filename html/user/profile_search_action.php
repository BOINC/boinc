<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

function show_profile_link($profile, $n) {
    $user = lookup_user_id($profile->userid);
    echo "<tr><td align=\"center\">".user_links($user)."</td><td align=\"center\">".date_str($user->create_time)."</td><td align=\"center\">".$user->country."</td><td align=\"center\">".(int)$user->total_credit."</td><td align=\"center\">".(int)$user->expavg_credit."</td></tr>\n";
}

db_init();

$search_string = get_str('search_string');
$offset = get_int('offset', true);
if (!$offset) $offset=0;
$count = 10;

page_head("Profile search results");

echo "<h2>Profiles containing '$search_string'</h2>\n";
$q = "select * from profile where match(response1, response2) against ('$search_string') limit $offset,$count";
$result = mysql_query($q);
echo "<table align=\"center\" cellpadding=\"1\" border=\"1\" width=\"90%\"><tr><th align=\"center\">User name</th><th align=\"center\">Joined project</th><th align=\"center\">Country</th><th  align=\"center\">Total credit</th><th  align=\"center\">Recent credit</th></tr>";
$n = 0;
while ($profile = mysql_fetch_object($result)) {
    show_profile_link($profile, $n+$offset+1);
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
