<?php
/**
 * This file gives moderators the ability to moderate threads, taking
 * action from forum_moderate_thread.php
 **/
 
require_once("../inc/forum.inc");
require_once("../inc/email.inc");
require_once("../inc/forum_std.inc");


db_init();
$logged_in_user = re_get_logged_in_user();

if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page("You must specify an action...");
    } else {
	$action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$thread = new Thread(get_int('thread'));

if (!$logged_in_user->isSpecialUser(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page("You are not authorized to moderate this post.");
}

if ($action=="hide"){
    // Hide the thread in the given hidden category
    $result = $thread->hide(post_int("category"));
} elseif ($action=="unhide"){
    $result = $thread->unhide();
} elseif ($action=="sticky"){
    $result = $thread->setSticky(true);
} elseif ($action=="desticky"){
    $result = $thread->setSticky(false);
} elseif ($action=="move"){
    $forum = new Forum(post_int('forumid'));
    $result = $thread->moveTo($forum);
} elseif ($action=="title"){
    $title = post_str('newtitle');
    $result = $thread->setTitle($title);
} else {
    error_page("Unknown action ");
}


if ($result) {
    if (post_str('reason', true)){
        send_thread_moderation_email($thread, post_str("reason"));
    }
    header('Location: forum_thread.php?id='.$thread->getID());
} else {
    error_page("Moderation failed");
}

?>
