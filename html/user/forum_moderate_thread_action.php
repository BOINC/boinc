<?php

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");

$logged_in_user = get_logged_in_user();
check_tokens($logged_in_user->authenticator);
BoincForumPrefs::lookup($logged_in_user);
if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page("You must specify an action...");
    } else {
        $action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$thread = BoincThread::lookup_id(get_int('thread'));
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($logged_in_user, $forum)) {
    error_page("You are not authorized to moderate this post.");
}

if ($action=="hide") {
    $cat = post_int("category");        // TODO - store this somewhere
    $result = hide_thread($thread, $forum);
} elseif ($action=="unhide"){
    $result = unhide_thread($thread, $forum);
} elseif ($action=="sticky"){
    $result = $thread->update("sticky=1");
} elseif ($action=="desticky"){
    $result = $thread->update("sticky=0");
} elseif ($action == "lock") {
    $result = $thread->update("locked=1");
} elseif ($action == "unlock") {
    $result = $thread->update("locked=0");
} elseif ($action=="move"){
    if ($forum->parent_type != 0) error_page("No");
    $fid = post_int('forumid');
    $new_forum = BoincForum::lookup_id($fid);
    $result = move_thread($thread, $forum, $new_forum);
} elseif ($action=="title"){
    $title = post_str('newtitle');
    $result = $thread->update("title='$title'");
} else {
    error_page("Unknown action ");
}

if ($result) {
    if (post_str('reason', true)){
        send_thread_moderation_email($thread, post_str("reason"),$action);
    } else {
        send_thread_moderation_email($thread, "None Given",$action);
    }
    header('Location: forum_thread.php?id='.$thread->id);
} else {
    error_page("Moderation failed");
}

?>
