<?php
    // show all the hosts for a user.
    // if $userid is absent, show hosts of logged-in user

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/host.inc");
    require_once("../inc/cache.inc");

    db_init();
    $userid = $_GET["userid"];
    if ($userid) {
        $cache_args = "userid=$userid";
	$caching=true;
        start_cache(USER_PAGE_TTL,$cache_args);
        $result = mysql_query("select * from user where id=$userid");
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        if ($user->show_hosts) {
            page_head("Computers belonging to $user->name");
            user_host_table_start(false);
        } else {
            echo "Hidden\n";
	    end_cache(USER_PAGE_TTL,$cache_args);
            exit();
        }
        $private = false;
    } else {
        $caching=false;
        $user = get_logged_in_user();
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
    page_tail();
    if ($caching) end_cache(USER_PAGE_TTL,$cache_args);
?>
