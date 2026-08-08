<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("action", "thread", "tnow", "ttok"));

$action = get_str('action');
$threadid = get_int('thread');
$thread = BoincThread::lookup_id($threadid);
if (!$thread) error_page('No such thread');
$forum = BoincForum::lookup_id($thread->forum);

function show_title($forum, $thread) {
    switch ($forum->parent_type) {
    case 0:
        $category = BoincCategory::lookup_id($forum->category);
        echo forum_title($category, $forum, $thread);
        break;
    case 1:
        echo team_forum_title($forum, $thread);
        break;
    }
}

function subscribe($forum, $thread, $user) {
    if (BoincSubscription::replace($user->id, $thread->id)) {
        page_head(tra("Subscription successful"));
        show_forum_header($user);
        show_title($forum, $thread);
        echo "<p>".tra("You are now subscribed to %1. You will be notified whenever there is a new post.", "<b>".cleanup_title($thread->title)."</b>");
    } else {
        page_head(tra("Subscription failed"));
        echo "<p>".tra("We are currently unable to subscribe you to %1. Please try again later..", "<b>".cleanup_title($thread->title)."</b>");
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->id."\">".tra("Return to thread")."</a></p>";
    page_tail();
}

function unsubscribe($forum, $thread, $user) {
    BoincSubscription::delete($user->id, $thread->id);
    if (!BoincSubscription::lookup($user->id, $thread->id)) {
        page_head(tra("Unsubscription successful"));
        show_forum_header($user);
        show_title($forum, $thread);
        echo "<p>".tra("You are no longer subscribed to %1. You will no longer receive notifications for this thread.", "<b>".cleanup_title($thread->title)."</b>");
    } else {
        page_head(tra("Unsubscription failed"));
        echo "<p>".tra("We are currently unable to unsubscribe you from %1. Please try again later..", "<b>".cleanup_title($thread->title)."</b>");
    }
    echo "</p><p><br /><a href=\"forum_thread.php?id=".$thread->id."\">".tra("Return to thread")."</a></p>";
    page_tail();
}

if (!$thread || !$action) {
    error_page(tra("Unknown subscription action"));
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

