<?php
// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/cache.inc");

function more_or_less($show_all) {
    if ($show_all) {
        echo "<p>Show: All hosts  |  ".link_with_GET_variables("Only hosts active in past 30 days<br>", "hosts_user.php", 'show_all', '0');
    } else {
        echo "<p>Show: ".link_with_GET_variables("All hosts", "hosts_user.php", 'show_all', '1')."  |  Only hosts active in past 30 days<br>";;
    }
}

// The following is used to show a user's hosts
//
function user_host_table_start($private) {
    start_table();
    echo "<tr>";
    echo "<th>".link_with_GET_variables("Computer ID", "hosts_user.php", 'sort', 'id')."<br><font size=-2>Click for more info</font></th>\n";
    if ($private) {
        echo "<th>".link_with_GET_variables("Name", "hosts_user.php", 'sort', 'name')."</th>\n
        ";
    } else {
        echo "<th>Rank</th>";
    }
    echo "
        <th>".link_with_GET_variables("Recent average credit", "hosts_user.php", 'sort', 'expavg_credit')."</th>
        <th>".link_with_GET_variables("Total credit", "hosts_user.php", 'sort', 'total_credit')."</th>
        <th>".link_with_GET_variables("CPU type", "hosts_user.php", 'sort', 'cpu')."</th>
        <th>".link_with_GET_variables("Operating system", "hosts_user.php", 'sort', 'os')."</th>
    ";
    $config = get_config();
    if (parse_bool($config, "show_results")) echo "<th>Results</th>";
    echo "<th>".link_with_GET_variables("Last contact", "hosts_user.php", 'sort', 'rpc_time')."</th>";
}


db_init();

// get the _GET variables which determine how to display the page
//
$userid = get_int("userid", true);
$show_all = get_int("show_all", true);
if (!$show_all) $show_all=0;
$sort = get_str("sort", true);
if (!$sort) $sort = "rpc_time";

$user = get_logged_in_user(false);
if ($user && $user->id == $userid) {
    $userid = 0;
}
if ($userid) {
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    $caching=true;
    $cache_args="userid=$userid&show_all=$show_all&sort=$sort";
    start_cache(USER_PAGE_TTL, $cache_args);
    if ($user->show_hosts) {
        page_head("Computers belonging to $user->name");
        more_or_less($show_all);
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
    more_or_less($show_all);
    user_host_table_start(true);
    $private = true;
}

$sort_clause = "rpc_time desc";
if ($sort == "total_credit") $sort_clause = "total_credit desc";
if ($sort == "total_credit_reversed") $sort_clause = "total_credit";
if ($sort == "expavg_credit") $sort_clause = "expavg_credit desc";
if ($sort == "expavg_credit_reversed") $sort_clause = "expavg_credit";
if ($sort == "name") $sort_clause = "domain_name";
if ($sort == "name_reversed") $sort_clause = "domain_name desc";
if ($sort == "id") $sort_clause = "id";
if ($sort == "id_reversed") $sort_clause = "id desc";
if ($sort == "expavg_credit") $sort_clause = "expavg_credit desc";
if ($sort == "expavg_credit_reversed") $sort_clause = "expavg_credit ";
if ($sort == "cpu") $sort_clause = "p_model";
if ($sort == "cpu_reversed") $sort_clause = "p_model desc";
if ($sort == "os") $sort_clause = "os_name, os_version";
if ($sort == "os_reversed") $sort_clause = "os_name desc, os_version";
if ($sort == "rpc_time") $sort_clause = "rpc_time desc"; 
if ($sort == "rpc_time_reversed") $sort_clause = "rpc_time"; 

$now = time();
$old_hosts=0;
$i = 1;
$result = mysql_query("select * from host where userid=$userid order by $sort_clause");
while ($host = mysql_fetch_object($result)) {
    $is_old=false;
    if (($now - $host->rpc_time) > 30*86400) {
        $is_old=true;
        $old_hosts++;
    }
    if (!$show_all && $is_old) continue;
    show_host_row($host, $i, $private, false);
    $i++;
}
mysql_free_result($result);
echo "</table>\n";

if ($old_hosts>0) more_or_less($show_all);

if ($caching) {
    page_tail(true);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    page_tail();
}
?>
