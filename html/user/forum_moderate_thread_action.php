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

$explanation = "";
$cat = post_int("category", true);
if ($cat) {
    $explanation .= "Reason: ";
    switch ($cat) {
    case 1: $explanation .= "obscene"; break;
    case 2: $explanation .= "flame/hate mail"; break;
    case 3: $explanation .= "commercial spam"; break;
    case 4: $explanation .= "other"; break;
    }
    $explanation .= "\n";
}

$comment = post_str('reason', true);
if ($comment) {
    $explanation .= "Moderator comment: $comment\n";
}

switch ($action) {
case "hide":
    $result = hide_thread($thread, $forum);
    $action_name = "hidden";
    break;
case "unhide":
    $result = unhide_thread($thread, $forum);
    $action_name = "unhidden";
    break;
case "sticky":
    $result = $thread->update("sticky=1");
    $action_name = "made sticky";
    break;
case "desticky":
    $result = $thread->update("sticky=0");
    $action_name = "made non-sticky";
    break;
case "lock":
    $result = $thread->update("locked=1");
    $action_name = "locked";
    break;
case "unlock":
    $result = $thread->update("locked=0");
    $action_name = "unlocked";
    break;
case "move":
    if ($forum->parent_type != 0) error_page("No");
    $fid = post_int('forumid');
    $new_forum = BoincForum::lookup_id($fid);
    $result = move_thread($thread, $forum, $new_forum);
    $action_name = "moved from $forum->title to $new_forum->title";
    break;
case "title":
    $title = process_user_text(post_str('newtitle'));
    $result = $thread->update("title='$title'");
    $action_name = "renamed from '$thread->title' to '$title'";
    break;
default:
    error_page("Unknown action ");
}

if (!$result) {
    error_page("Moderation failed");
}

$reason = post_str('reason', true);
if (!$reason) $reason = "None given";
send_thread_moderation_email(
    $forum, $thread, $reason, $action_name, $explanation
);
header('Location: forum_thread.php?id='.$thread->id);

?>
