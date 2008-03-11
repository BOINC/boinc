<?php

// show all the hosts for a user.
// if $userid is absent, show hosts of logged-in user

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/host.inc");
require_once("../inc/cache.inc");

// This function takes as input a GET variable name X and value Y.
// It returns the corresponding text GET variable string, appended with
// any existing GET variable names and values, with &X=Y.
//
// This is useful for constructing urls for sorting a table by different
// columns.
//
function make_GET_list($variable_name, $variable_value) {
    $retval="";
    $sepchar='?';
    $modified=false;
    foreach ($_GET as $key => $value) {
        $retval .= "$sepchar"."$key=";
        $sepchar='&';
        if ($key==$variable_name) {
            $modified=true;
            if ($value!=$variable_value) {
                $retval .= "$variable_value";
            } else {
                $retval .= "$variable_value"."_reversed";
            }
        }
        else {
            $retval .= "$value";
        }
    }
    if (!$modified) $retval .= "$sepchar$variable_name=$variable_value";
    return $retval;
}

function link_with_GET_variables(
    $text, $baseurl, $variable_name, $variable_value
) {
    $list=make_GET_list($variable_name, $variable_value);
    return "<a href=\"$baseurl$list\">$text</a>";
}

function more_or_less($show_all) {
    if ($show_all) {
        echo "<p>Show: All computers | ".link_with_GET_variables("Only computers active in past 30 days<p>", "hosts_user.php", 'show_all', '0');
    } else {
        echo "<p>Show: ".link_with_GET_variables("All computers", "hosts_user.php", 'show_all', '1')." | Only computers active in past 30 days<p>";;
    }
}

// The following is used to show a user's hosts
//
function user_host_table_start($private) {
    start_table();
    echo "<tr>";
    echo "<th>".link_with_GET_variables("Computer ID", "hosts_user.php", 'sort', 'id')."<br><font size=-2>Click for more info</font></th>\n";
    if ($private) {
        echo "<th>".link_with_GET_variables("Name", "hosts_user.php", 'sort', 'name')."</th>\n";
        echo "<th>".link_with_GET_variables("Location", "hosts_user.php", 'sort', 'venue')."</th>\n";
    } else {
        echo "<th>Rank</th>";
    }
    echo "
        <th>".link_with_GET_variables("Avg. credit", "hosts_user.php", 'sort', 'expavg_credit')."</th>
        <th>".link_with_GET_variables("Total credit", "hosts_user.php", 'sort', 'total_credit')."</th>
        <th>".link_with_GET_variables("CPU type", "hosts_user.php", 'sort', 'cpu')."</th>
        <th>".link_with_GET_variables("Operating system", "hosts_user.php", 'sort', 'os')."</th>
    ";
    echo "<th>".link_with_GET_variables("Last contact", "hosts_user.php", 'sort', 'rpc_time')."</th>";
}

// get the _GET variables which determine how to display the page
//
$show_all = get_int("show_all", true);
if ($show_all != 1) {
    // default value -- show last 30 days
    $show_all = 0;
    $_GET['show_all'] = 0;
}

$sort = get_str("sort", true);
switch ($sort) {
case "total_credit": $sort_clause = "total_credit desc"; break;
case "total_credit_reversed": $sort_clause = "total_credit"; break;
case "expavg_credit": $sort_clause = "expavg_credit desc"; break;
case "expavg_credit_reversed": $sort_clause = "expavg_credit"; break;
case "name": $sort_clause = "domain_name"; break;
case "name_reversed": $sort_clause = "domain_name desc"; break;
case "id": $sort_clause = "id"; break;
case "id_reversed": $sort_clause = "id desc"; break;
case "expavg_credit": $sort_clause = "expavg_credit desc"; break;
case "expavg_credit_reversed": $sort_clause = "expavg_credit "; break;
case "cpu": $sort_clause = "p_vendor"; break;
case "cpu_reversed": $sort_clause = "p_vendor desc"; break;
case "os": $sort_clause = "os_name"; break;
case "os_reversed": $sort_clause = "os_name desc"; break;
case "venue": $sort_clause = "venue"; break;
case "venue_reversed": $sort_clause = "venue desc"; break;
case "rpc_time_reversed": $sort_clause = "rpc_time"; break;
default:
    // default value -- sort by RPC time
    $sort = "rpc_time";
    $sort_clause = "rpc_time desc"; 
    $_GET['sort']=$sort;
}

$user = get_logged_in_user(false);
$userid = get_int("userid", true);

if ($user && $user->id == $userid) {
    $userid = 0;
}
if ($userid) {
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    $caching = true;

    // At this point, we know that $userid, $show_all and $sort all have
    // valid values.
    //
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
    $caching = false;
    $userid = $user->id;
    page_head("Your computers");
    more_or_less($show_all);
    user_host_table_start(true);
    $private = true;
}

$now = time();
$old_hosts=0;
$i = 1;
$hosts = BoincHost::enum("userid=$userid order by $sort_clause");
foreach ($hosts as $host) {
    $is_old=false;
    if (($now - $host->rpc_time) > 30*86400) {
        $is_old=true;
        $old_hosts++;
    }
    if (!$show_all && $is_old) continue;
    show_host_row($host, $i, $private, false);
    $i++;
}
echo "</table>\n";

if ($old_hosts>0) {
    more_or_less($show_all);
}

if ($private) {
    echo "
        <a href=merge_by_name.php>Merge computers by name</a>
    ";
}

if ($caching) {
    page_tail(true);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    page_tail();
}

?>
