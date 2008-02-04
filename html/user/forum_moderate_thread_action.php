<?php

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");

$logged_in_user = get_logged_in_user();
check_tokens($logged_in_user->authenticator);
BoincForumPrefs::lookup($logged_in_user);
$action = post_str('action', true);
if (!$action) {
    $action = get_str('action');
}

$thread = BoincThread::lookup_id(get_int('thread'));
if (!$thread) error_page("no thread");
$forum = BoincForum::lookup_id($thread->forum);
if (!$forum) error_page("no forum");

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
    $reason = post_str('reason', true);
    if (!$reason) $reason = "None given";
    send_thread_moderation_email($forum, $thread, $reason, $action);
    header('Location: forum_thread.php?id='.$thread->id);
} else {
    error_page("Moderation failed");
}

?>
