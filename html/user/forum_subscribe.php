<?php

require_once('../inc/subscribe.inc');
require_once('../inc/util.inc');

db_init();

$action = $_GET['action'];
$thread = $_GET['thread'];

if ($thread && $action) {
    $result = mysql_query("SELECT * FROM thread WHERE id = $thread");
    $thread = mysql_fetch_object($result);
    $user = get_logged_in_user($true);
    
    if ($action == "subscribe") {
        subscribe($thread, $user);
        exit();
    } else if ($action == "unsubscribe") {
        unsubscribe($thread, $user);
        exit();
    } else {
        show_result_page(null, false, $thread);
        exit();
    }
} else {
    show_result_page(null, false, NULL);
}

?>

