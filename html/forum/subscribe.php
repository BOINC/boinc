<?php
require_once('subscribe.inc');
require_once('../util.inc');

$thread = $_GET['thread'];

if ($thread) {
    $result = sql_query("SELECT * FROM thread WHERE id = $thread");
    $thread = mysql_fetch_object($result);
    $user = get_logged_in_user($true);
    subscribe($thread, $user);
} else {
    show_result_page(false, NULL);
}
?>