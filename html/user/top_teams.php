<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$n = 20;

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}
$offset = get_int("offset", true);
if (!$offset) $offset=0;
$type = get_int("type", true);
if ($type < 1 || $type > 7) {
    $type = null;
}
$type_url="";
$type_sql="";
$type_name="";

if ($type) {
    $type_url = "&type=$type";
    $type_sql = "where type=$type";
    $type_name = team_type_name($type);
}

if ($offset % $n) $offset = 0;

if ($offset < 1000) {
    $cache_args = "sort_by=$sort_by&offset=$offset".$type_url;
    start_cache(TOP_PAGES_TTL, $cache_args);
} else {
    page_head("Limit exceeded");
    echo "Sorry - first 1000 only.";
    page_tail();
    exit();
}

require_once("../inc/db.inc");
require_once("../inc/user.inc");

db_init();

page_head("Top $type_name teams");
if ($sort_by == "total_credit") {
    $sort_clause = "total_credit desc";
} else {
    $sort_clause = "expavg_credit desc";
}
$result = mysql_query("select * from team $type_sql order by $sort_clause limit $offset,$n");
start_table();
row1("Teams", 6);
team_table_start($sort_by);
$i = $offset + 1;
while ($team = mysql_fetch_object($result)) {
    if ($sort_by == "total_credit") {
        show_team_row($team, $i);
        $i++;
    } else {
        if (!team_inactive_ndays($team, 7)) {
            show_team_row($team, $i);
            $i++;
        }
    }
}
mysql_free_result($result);
echo "</table>\n<p>\n";
if ($offset > 0) {
    $new_offset = $offset - $n;
    echo "<a href=top_teams.php?sort_by=$sort_by&offset=$new_offset".$type_url.">Last $n</a> | ";

}
$new_offset = $offset + $n;
echo "<a href=top_teams.php?sort_by=$sort_by&offset=$new_offset".$type_url.">Next $n</a>";

if ($offset < 1000) {
    page_tail(true);
    end_cache(TOP_PAGES_TTL, $cache_args);
} else {
    page_tail();
}

?>
