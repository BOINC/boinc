<?php
    require_once("../inc/cache.inc");
    require_once("../inc/util.inc");

    $id = get_int("userid");
    $format = get_str("format", true);
    $cache_args = "userid=$id";
    if ($format=="xml") {
        $cache_args .= "&format=xml";
    }
    start_cache(USER_PAGE_TTL, $cache_args);

    require_once("../inc/db.inc");
    require_once("../inc/user.inc");
    require_once("../inc/forum.inc");
    db_init();

    $result = mysql_query("select * from user where id = $id");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);

    $user = getForumPreferences($user);

    if ($format == "xml") {
        require_once("../inc/xml.inc");
        if ($user) {
            show_user_xml($user);
        } else {
            xml_error("no such user ID");
        }
    } else {
        if ($user) {
            page_head("Account data for $user->name");
            start_table();
            show_user_summary_public($user);
            end_table();
            page_tail(true);
        } else {
            page_head("Can't find user");
            echo "There is no account with that ID.\n<p>";
            page_tail(true);
        }
    }
    end_cache(USER_PAGE_TTL,$cache_args);
?>
