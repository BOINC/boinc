<?php
// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/cache.inc");

db_init();
$userid = get_int("userid", true);
$show_all = get_int("show_all", true);
if (!$show_all) $show_all = 0;

$user = get_logged_in_user(false);
if ($user && $user->id == $userid) {
    $userid = 0;
}
if ($userid) {
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    $cache_args = "userid=$userid&show_all=$show_all";
    $caching=true;
    start_cache(USER_PAGE_TTL, $cache_args);
    if ($user->show_hosts) {
        page_head("Computers belonging to $user->name");
        user_host_table_start(false);
    } else {
        page_head("Computers hidden");
        echo "This user has chosen not to show information about their computers.\n";
        page_tail();
        end_cache(USER_PAGE_TTL, $cache_args);
        exit();
    }
    $private = false;
} else {
    $user = get_logged_in_user();
    $caching=false;
    $userid = $user->id;
    page_head("Your computers");
    user_host_table_start(true);
    $private = true;
}
$i = 1;

$sort_clause = "rpc_time desc";
$sort = get_str("sort", true);
if ($sort == "total_credit") $sort_clause = "total_credit desc";
if ($sort == "expavg_credit") $sort_clause = "expavg_credit desc";

$now = time();
$more_hosts = false;

$result = mysql_query("select * from host where userid=$userid order by $sort_clause");
while ($host = mysql_fetch_object($result)) {
    if (!$show_all && (($now - $host->rpc_time) > 30*86400)) {
        $more_hosts = true;
        continue;
    }
    show_host_row($host, $i, $private, false);
    $i++;
}
mysql_free_result($result);
echo "</table>\n";
if ($more_hosts) {
    echo "<p>
        Hosts older than 30 days not shown.
        <a href=hosts_user.php?userid=$userid&show_all=1>Show all hosts</a>.
    ";
}
if ($caching) {
    page_tail(true);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    page_tail();
}
?>
