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
    $new_title = post_str('newtitle');
    $title = BoincDb::escape_string($new_title);
    $result = $thread->update("title='$title'");
    $action_name = "renamed from '$thread->title' to '$new_title'";
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
