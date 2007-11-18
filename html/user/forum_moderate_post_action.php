<?php

// When a moderator does something to a post, this page actually
// commits those changes to the database.

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");

$user = get_logged_in_user();
check_tokens($user->authenticator);
BoincForumPrefs::lookup($user);
$post = BoincPost::lookup_id(get_int('id'));
$thread = BoincThread::lookup_id($post->thread);
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($user, $forum)) {
    error_page("You are not authorized to moderate this post.");
}

// See if "action" is provided - either through post or get
if (!post_str('action', true)) {
    if (!get_str('action', true)){
        error_page("You must specify an action...");
    } else {
        $action = get_str('action');
    }
} else {
    $action = post_str('action');
}

if ($action=="hide"){
    $result = hide_post($post, $thread, $forum);
} elseif ($action=="unhide"){
    $result = unhide_post($post, $thread, $forum);
} elseif ($action=="move"){
    $destid = post_int('threadid');
    $new_thread = BoincThread::lookup_id($destid);
    $new_forum = BoincForum::lookup_id($new_thread->forum);
    if ($forum->parent_type != $new_forum->parent_type) {
        error_page("Can't move to different category type");
    }
    if ($forum->parent_type != 0) {
        if ($forum->category != $new_forum->category) {
            error_page("Can't move to different category");
        }
    }
    $result = move_post($post, $thread, $forum, $new_thread, $new_forum);
} elseif ($action=="banish_user"){
    if (!$user->prefs->privilege(S_ADMIN)) {
      // Can't banish without being administrator
        error_page("You are not authorized to banish this user.");
    }
    $userid = post_int('userid');
    $user = BoincUser::lookup_id($userid);
    if (!$user) {
        error_page("no user");
    }
    BoincForumPrefs::lookup($user);
    $duration = post_int('duration');
    if ($duration == -1) {
        $t = 2147483647; // Maximum integer value
    } else {
        $t = time() + $duration;
    }
    $reason = post_str("reason", true);
    $result = $user->prefs->update("banished_until=$t");
    page_head("Banishment");
    if ($result) {
        echo "User ".$user->name." has been banished.";
        send_banish_email($forum, $user, $t, $reason);
    } else {
        echo "DB failure";
    }
    page_tail();
    exit();
} else {
    error_page("Unknown action ");
}

switch (post_int("category", true)) {
case 1:
    $mod_category = "Obscene";
    break;
case 2:
    $mod_category = "Flame/Hate mail";
    break;
case 3:
    $mod_category = "Commercial spam";
    break;
case 4:
    $mod_category = "Doublepost";
    break;
case 5:
    $mod_category = "User Request";
    break;
default:
    $mod_category = "Other";
    break;
}

if ($result) {
    if (post_str('reason', true)){
        send_moderation_email($forum, $post, $thread, "Category: ".$mod_category."\n".post_str("reason"), $action);
    } else { 
        send_moderation_email($forum, $post, $thread, "Category: ".$mod_category."\n"."None given", $action);
    }
    header('Location: forum_thread.php?id='.$thread->id);
} else {
    error_page("Moderation failed");
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
