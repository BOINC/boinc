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

// This file allows people to subscribe to threads.
// Whenever someone posts to the thread
// the subscribers will receive a notification email

require_once('../inc/forum.inc');

$action = get_str('action');
$threadid = get_int('thread');
$thread = BoincThread::lookup_id($threadid);
$forum = BoincForum::lookup_id($thread->forum);

function show_title($forum, $thread) {
    switch ($forum->parent_type) {
    case 0:
        $category = BoincCategory::lookup_id($forum->category);
        show_forum_title($category, $forum, $thread);
        break;
    case 1:
        show_team_forum_title($forum, $thread);
        break;
    }
}

function subscribe($forum, $thread, $user) {
    if (BoincSubscription::replace($user->id, $thread->id)) {
        page_head("Subscription successful");
        show_forum_header($user);
        show_title($forum, $thread);
        echo "<p>You are now subscribed to <b>", cleanup_title($thread->title), "</b>.
        You will be notified whenever there is a new post.";
    } else {
        page_head("Subscription failed");
        echo "<p>We are currently unable to subscribe you to this thread (<b>", cleanup_title($thread->title), "</b>).
        Please try again later..";
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->id."\">Return to thread</a></p>";
    page_tail();
}

function unsubscribe($forum, $thread, $user) {
    BoincSubscription::delete($user->id, $thread->id);
    if (!BoincSubscription::lookup($user->id, $thread->id)) {
        page_head("Unsubscription successful");
        show_forum_header($user);
        show_title($forum, $thread);
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

if (!$thread || !$action) {
    error_page("Unknown subscription action");
}

$user = get_logged_in_user();
check_tokens($user->authenticator);

if ($action == "subscribe") {
    subscribe($forum, $thread, $user);
    exit();
} else if ($action == "unsubscribe") {
    unsubscribe($forum, $thread, $user);
    exit();
}

?>

