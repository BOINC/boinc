<?php
    // show all the hosts for a user.
    // if $userid is absent, show hosts of logged-in user

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/host.inc");
    require_once("../inc/cache.inc");

    db_init();
    $userid = get_int("userid", true);
    $user = get_logged_in_user(false);
    if ($user && $user->id == $userid) {
        $userid = 0;
    }
    if ($userid) {
        $user = lookup_user_id($userid);
        if (!$user) {
            error_page("No such user");
        }
        $cache_args = "userid=$userid";
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
        $caching=false;
        $userid = $user->id;
        page_head("Your computers");
        user_host_table_start(true);
        $private = true;
    }
    $i = 1;
    $result = mysql_query("select * from host where userid=$userid order by expavg_credit desc");
    while ($host = mysql_fetch_object($result)) {
        show_host_row($host, $i, $private, false);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    if ($caching) {
        page_tail(true);
        end_cache(USER_PAGE_TTL, $cache_args);
    } else {
        page_tail();
    }
?>
