<?php
/**
 * This page shows basic account details about a given user
 * The page can be output as either XML or HTML. If output as xml the user may optionally
 * also get a list of hosts in case the user provides his/her authenticator.
 * Object-caching and full-file caching is used to speed up queries for data from this page.
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/db.inc");
require_once("../inc/user.inc");
require_once("../inc/forum.inc");


$id = get_int("userid", true);
$format = get_str("format", true);
$auth = get_str("auth", true);

if ($format=="xml"){
    // XML doesn't need translating, so let's use the full-file cache for this
    $cache_args="userid=".$id."&auth=".$auth;
    start_cache(USER_PAGE_TTL, $cache_args);
    db_init();
    if ($auth){
        $user = lookup_user_auth($auth);        
        $show_hosts = true;
    } else {
        $user = lookup_user_id($id);
        $show_hosts = false;
    }
    if (!$user) xml_error("no such user ID");

    // Output:
    show_user_xml($user, $show_hosts);
    end_cache(USER_PAGE_TTL,$cache_args);
} else {
    // The webfrontend may be presented in many different languages, so here we cache the data instead
    $cache_args="userid=".$id;
    $cached_data = get_cached_data(TOP_PAGES_TTL,$cache_args);
    if ($cached_data){
        // We found some old but non-stale data, let's use it
        $user = unserialize($cached_data);
    } else {
        // No data was found, generate new data for the cache and store it
        db_init();
        $user = lookup_user_id($id);
        $user = getForumPreferences($user);
        set_cache_data(serialize($user),$cache_args);
    }
    if (!$user->id) error_page("No such user found - please check the ID and try again. If the user is very new it may take a while before it can be displayed.");

    // Output:
    page_head("Account data for $user->name");
    start_table();
    show_user_summary_public($user);
    show_profile_link($user);
    end_table();
    project_user_summary($user);
    page_tail(true);
}

?>
