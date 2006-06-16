<?php
/**
 * This file allows people to subscribe to threads
 * Whenever someone posts a new post to the thread
 * the subscribers will receive a notification email
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');

db_init();

$action = get_str('action');
$threadid = get_int('thread');
$thread = new Thread($threadid);


function subscribe($thread, $user) {
    $thread->subscribe($user);
    if ($thread->isSubscribed($user)){
	page_head("Subscription Successful");
	show_forum_title($thread->getForum(), $thread);
        echo "<p>You are now subscribed to <b>", cleanup_title($thread->getTitle()), "</b>.
	You will receive an email whenever someone posts to the thread.";
    } else {
	page_head("Subscription failed");
        echo "<p>We are currently unable to subscribe you to this thread (<b>", cleanup_title($thread->getTitle()), "</b>).
	Please try again later..";
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->getID()."\">Return to thread</a></p>";
    page_tail();
}

function unsubscribe($thread, $user=null) {
    $thread->unsubscribe($user);
    if (!$thread->isSubscribed($user)){
	page_head("Unsubscription Successful");
	show_forum_title($thread->getForum(), $thread);
        echo "<p>You are no longer subscribed to <b>", cleanup_title($thread->getTitle()), "</b>.
	You will no longer receive notifications for this thread.";
    } else {
	page_head("Unsubscription failed");
        echo "<p>We are currently unable to unsubscribe you to this thread (<b>", cleanup_title($thread->getTitle()), "</b>).
	Please try again later..";
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->getID()."\">Return to thread</a></p>";
    page_tail();
}

if ($thread && $action) {
    $user = re_get_logged_in_user(true);
    
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
    error_page("Unknown subscription action");
}

?>

