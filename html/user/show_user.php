<?php
    require_once("../inc/cache.inc");
    require_once("../inc/util.inc");

    $id = get_int("userid");
    $format = get_str("format", true);

    require_once("../inc/db.inc");
    require_once("../inc/user.inc");
    require_once("../inc/forum.inc");
    db_init();

    $user = lookup_user_id($id);
    if (!$user) {
        error_page("no such user");
    }

    $cache_args = "userid=$id";
    if ($format=="xml") {
        $cache_args .= "&format=xml";
    }
    start_cache(USER_PAGE_TTL, $cache_args);

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
	        project_user_summary($user);
            page_tail(true);
        } else {
            page_head("Can't find user");
            echo "There is no account with that ID.\n<p>";
            page_tail(true);
        }
    }
    end_cache(USER_PAGE_TTL,$cache_args);
?>
