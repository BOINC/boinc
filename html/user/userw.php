<?php
    require_once("../inc/util.inc");
    require_once("../inc/userw.inc");
    require_once("../inc/db.inc");
    require_once("../inc/wap.inc");
    require_once("../inc/cache.inc");
    
    $userid = get_int('id');
    
    $cache_args = "userid=".$userid;
    start_cache(USER_PAGE_TTL, $cache_args);
    
    db_init();
    
    $user = lookup_user_id($userid);
    if (!$user) {
        error_page("No such user");
    }
    show_user_wap($user);
    
    end_cache(USER_PAGE_TTL, $cache_args);
?>
