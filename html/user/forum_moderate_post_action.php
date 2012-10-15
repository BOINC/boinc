<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");

check_get_args(array("id", "action", "tnow", "ttok"));

function mod_comment() {
    $x = "";
    $reason = post_str('reason', true);
    if ($reason){
        $x .= "\nModerator comment: $reason
";
    }
    return $x;
}

function hide_explanation() {
    switch (post_int("category", true)) {
        case 1: $c = "Obscene"; break;
        case 2: $c = "Flame/Hate mail"; break;
        case 3: $c = "Commercial spam"; break;
        case 4: $c = "Double post"; break;
        case 5: $c = "User Request"; break;
        default: $c = "Other"; break;
    }
    $x = "\nYour post was categorized as ".$c;
    $x .= mod_comment();
    return $x;
}

$user = get_logged_in_user();
check_tokens($user->authenticator);
BoincForumPrefs::lookup($user);
$post = BoincPost::lookup_id(get_int('id'));
if (!$post) error_page("no such post");
$thread = BoincThread::lookup_id($post->thread);
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($user, $forum)) {
    error_page(tra("You are not authorized to moderate this post."));
}

// See if "action" is provided - either through post or get
if (!post_str('action', true)) {
    if (!get_str('action', true)){
        error_page(tra("You must specify an action..."));
    } else {
        $action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$explanation = null;
if ($action=="hide"){
    $result = hide_post($post, $thread, $forum);
    $action_name = "hidden";
    $explanation = hide_explanation();
} elseif ($action=="unhide"){
    $result = unhide_post($post, $thread, $forum);
    $action_name = "unhidden";
} elseif ($action=="move"){
    $destid = post_int('threadid');
    $new_thread = BoincThread::lookup_id($destid);
    if (!$new_thread) error_page("No such thread");
    $new_forum = BoincForum::lookup_id($new_thread->forum);
    if ($forum->parent_type != $new_forum->parent_type) {
        error_page(tra("Can't move to different category type"));
    }
    if ($forum->parent_type != 0) {
        if ($forum->category != $new_forum->category) {
            error_page(tra("Can't move to different category"));
        }
    }
    $result = move_post($post, $thread, $forum, $new_thread, $new_forum);
    $explanation = "Old thread: $thread->title
".URL_BASE."forum_thread.php?id=$thread->id
New thread: $new_thread->title
".URL_BASE."forum_thread.php?id=$new_thread->id&postid=$post->id
";
    $explanation .= mod_comment();
    $action_name = "moved to another thread";
} elseif ($action=="banish_user"){
    $auth = false;
    if (defined("MODERATORS_CAN_BANISH") && $user->prefs->privilege(S_MODERATOR)) {
        $auth = true;
    } else {
        if ($user->prefs->privilege(S_ADMIN)) {
            $auth = true;
        }
    }
    if (!$auth) {
        error_page(tra("Not authorized to banish users"));
    }
    $userid = post_int('userid');
    $bad_user = BoincUser::lookup_id($userid);
    if (!$bad_user) {
        error_page("user ID not found");
    }
    BoincForumPrefs::lookup($bad_user);
    $duration = post_int('duration');
    if ($duration == -1) {
        $t = 2147483647; // Maximum integer value
    } else {
        $t = time() + $duration;
    }
    $reason = post_str("reason", true);
    $result = $bad_user->prefs->update("banished_until=$t");
    page_head(tra("Banishment"));
    if ($result) {
        echo tra("User %1 has been banished.", $bad_user->name);
        send_banish_email($forum, $bad_user, $t, $reason);
    } else {
        error_page(tra("Action failed: possible database problem"));
    }
    page_tail();
    exit();
} else {
    error_page("Unknown action");
}

if (!$result) {
    error_page("Action failed: possible database problem");
}

send_moderation_email($forum, $post, $thread, $explanation, $action_name);

header('Location: forum_thread.php?id='.$thread->id);

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
