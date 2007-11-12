<?php
// This file allows people to subscribe to threads
// Whenever someone posts a new post to the thread
// the subscribers will receive a notification email

require_once('../inc/forum.inc');

$action = get_str('action');
$threadid = get_int('thread');
$thread = BoincThread::lookup_id($threadid);
$forum = BoincForum::lookup_id($thread->forum);
$category = BoincCategory::lookup_id($forum->category);

function subscribe($category, $forum, $thread, $user) {
    if (BoincSubscription::replace($user->id, $thread->id)) {
        page_head("Subscription Successful");
        show_forum_title($user, $category, $forum, $thread, true);
        echo "<p>You are now subscribed to <b>", cleanup_title($thread->title), "</b>.
        You will receive an email whenever someone posts to the thread.";
    } else {
        page_head("Subscription failed");
        echo "<p>We are currently unable to subscribe you to this thread (<b>", cleanup_title($thread->title), "</b>).
        Please try again later..";
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->id."\">Return to thread</a></p>";
    page_tail();
}

function unsubscribe($category, $forum, $thread, $user) {
    BoincSubscription::delete($user->id, $thread->id);
    if (!BoincSubscription::lookup($user->id, $thread->id)) {
        page_head("Unsubscription Successful");
        show_forum_title($user, $category, $forum, $thread, true);
        echo "<p>You are no longer subscribed to <b>", cleanup_title($thread->title), "</b>.
        You will no longer receive notifications for this thread.";
    } else {
        page_head("Unsubscription failed");
        echo "<p>We are currently unable to unsubscribe you to this thread (<b>", cleanup_title($thread->title), "</b>).
        Please try again later..";
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->id."\">Return to thread</a></p>";
    page_tail();
}

if ($thread && $action) {
    $user = get_logged_in_user();
    check_tokens($user->authenticator);
    
    if ($action == "subscribe") {
        subscribe($category, $forum, $thread, $user);
        exit();
    } else if ($action == "unsubscribe") {
        unsubscribe($category, $forum, $thread, $user);
        exit();
    } else {
        show_result_page(null, false, $thread);
        exit();
    }
} else {
    error_page("Unknown subscription action");
}

?>

