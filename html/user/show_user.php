<?php
    require_once("../inc/cache.inc");
    require_once("../inc/util.inc");
    require_once("../inc/xml.inc");

    $id = get_int("userid", true);
    $format = get_str("format", true);
    $auth = get_str("auth", true);

    require_once("../inc/db.inc");
    require_once("../inc/user.inc");
    require_once("../inc/forum.inc");
    db_init();

    $show_hosts = false;
    if ($id) {
        $user = lookup_user_id($id);
    } else if (strlen($auth)) {
        $user = lookup_user_auth($auth);
        $show_hosts = true;
    }
    if (!$user) {
        if ($format == "xml") {
            xml_error("no such user ID");
        } else {
            error_page("no such user");
        }
    }

    if ($id) {
        $cache_args = "userid=$id";
        if ($format=="xml") {
            $cache_args .= "&format=xml";
        }
        start_cache(USER_PAGE_TTL, $cache_args);
    }

    $user = getForumPreferences($user);

    if ($format == "xml") {
        show_user_xml($user, $show_hosts);
    } else {
        page_head("Account data for $user->name");
        start_table();
        show_user_summary_public($user);
        show_profile_link($user);
        end_table();
        project_user_summary($user);
        page_tail(true);
    }
    if ($id) {
        end_cache(USER_PAGE_TTL,$cache_args);
    }
?>
