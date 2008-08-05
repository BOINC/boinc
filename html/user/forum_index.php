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

// Forum index
// shows the categories and the forums in each category

require_once('../inc/forum.inc');
require_once('../inc/pm.inc');
require_once('../inc/time.inc');

$user = get_logged_in_user(false);

// Process request to mark all posts as read
//
if ((get_int("read", true) == 1)) {
    if ($user) {
        check_tokens($user->authenticator);
        BoincForumPrefs::lookup($user);
        $now = time();
        $user->prefs->update("mark_as_read_timestamp=$now");
        Header("Location: ".get_str("return", true));
    }
}

function show_forum_summary($forum) {
    switch ($forum->parent_type) {
    case 0:
        $t = $forum->title;
        $d = $forum->description;
        break;
    case 1:
        $team = BoincTeam::lookup_id($forum->category);
        $t = $forum->title;
        if (!strlen($t)) $t = $team->name;
        $d = $forum->description;
        if (!strlen($d)) $d = "Discussion among members of $team->name";
        break;
    }
    echo "
        <tr class=\"row1\">
        <td>
            <em>
            <a href=\"forum_forum.php?id=$forum->id\">$t
            </a></em>
            <br><span class=\"smalltext\">$d</span>
        </td>
        <td>$forum->threads</td>
        <td>$forum->posts</td>
        <td>".time_diff_str($forum->timestamp, time())."</td>
    </tr>";
}

page_head(tra("%1 Message boards", PROJECT));

echo "
    <p>
    If you have a question or problem, please use the
    <a href=forum_help_desk.php>Questions & answers</a>
    area instead of the Message boards.
    </p>
";

show_forum_header($user);

$categories = BoincCategory::enum("is_helpdesk=0 order by orderID");
$first = true;
foreach ($categories as $category) {
    if ($first) {
        $first = false;
        show_forum_title($category, NULL, NULL);
        show_mark_as_read_button($user);
        echo "<p>";
        start_forum_table(
            array(tra("Topic"), tra("Threads"), tra("Posts"), tra("Last post"))
        );
    }
    if (strlen($category->name)) {
        echo '
            <tr class="subtitle">
            <td class="category" colspan="4">'.$category->name.'</td>
            </tr>
        ';
    }
    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        show_forum_summary($forum);
    }
}

if ($user && $user->teamid) {
    $forum = BoincForum::lookup("parent_type=1 and category=$user->teamid");
    if ($forum) {
        show_forum_summary($forum);
    }
}
end_table();

if ($user) {
    $subs = BoincSubscription::enum("userid=$user->id");
    if (count($subs)) {
        echo "<h3>Subscribed threads</h2>";
        show_thread_and_context_header();
        foreach ($subs as $sub) {
            $thread = BoincThread::lookup_id($sub->threadid);
            if (!$thread) {
                BoincSubscription::delete($user->id, $sub->threadid);
                continue;
            }
            if ($thread->hidden) continue;
            show_thread_and_context($thread, $user);
        }
        end_table();
    }
}

page_tail();
flush();
BoincForumLogging::cleanup();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
