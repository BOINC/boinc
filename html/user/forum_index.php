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

// Forum index
// shows the categories and the forums in each category

require_once('../inc/forum.inc');
require_once('../inc/pm.inc');
require_once('../inc/time.inc');

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
        if (!strlen($d)) $d = tra("Discussion among members of %1", $team->name);
        break;
    }
    echo "
        <tr>
        <td>
            <a href=\"forum_forum.php?id=$forum->id\">$t</a>
            <br><small>$d</small>
        </td>
        <td>$forum->threads</td>
        <td>$forum->posts</td>
        <td>".time_diff_str($forum->timestamp, time())."</td>
    </tr>";
}

function main($user) {
    page_head(tra("Message boards"));

    show_forum_header($user);

    if (function_exists('project_forum_index_intro')) {
        project_forum_index_intro();
    }

    if (defined('FORUM_QA_MERGED_MODE') && FORUM_QA_MERGED_MODE){
        $categories = BoincCategory::enum("true order by orderID");
    } else {
        echo "<p>"
            .tra("If you have a question or problem, please use the %1 Questions & Answers %2 section of the message boards.", "<a href=\"forum_help_desk.php\">", "</a>")
            ."</p>"
        ;
        $categories = BoincCategory::enum("is_helpdesk=0 order by orderID");
    }
    $first = true;
    foreach ($categories as $category) {
        if ($first) {
            $first = false;
            echo "<p>";
            echo forum_title($category, NULL, NULL);
            echo "<p>";
            show_mark_as_read_button($user);
            start_table('table-striped');
            row_heading_array(
                array(
                    tra("Forum"),
                    tra("Threads"),
                    tra("Posts"),
                    tra("Last post")
                ),
                array("width=30%", "", "", "")
            );
        }
        if (strlen($category->name)) {
            echo '
                <tr>
                <th class="info" colspan="4">'.$category->name.'</th>
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

    // show user's subscriptions (threads and forums)

    if ($user) {
        $subs = BoincSubscription::enum("userid=$user->id");
        $thread_subs = [];
        $forum_subs = [];
        foreach ($subs as $sub) {
            if ($sub->threadid > 0) {
                $thread_subs[] = $sub;
            } else {
                $forum_subs[] = $sub;
            }
        }
        if (count($subs)) {
            echo '<form action=forum_index.php method=post>
                <input type=hidden name=action value=unsub>
            ';
            echo "<p><h3>".tra("Subscriptions")."</h3><p>";
            start_table('table-striped');
        }
        if (count($thread_subs)) {
            thread_list_header(true);
            foreach ($thread_subs as $sub) {
                $thread = BoincThread::lookup_id($sub->threadid);
                if (!$thread) {
                    BoincSubscription::delete($user->id, $sub->threadid);
                    continue;
                }
                if ($thread->hidden) continue;
                thread_list_item($thread, $user, true);
            }
        }
        if (count($forum_subs)) {
            row_heading_array([
                'Forum', '', 'Last post', 'Unsubscribe'
            ]);
            foreach ($forum_subs as $sub) {
                $id = -$sub->threadid;
                $forum = BoincForum::lookup_id($id);
                if (!$forum) {
                    BoincSubscription::delete($user->id, $sub->threadid);
                    continue;
                }
                row_array([
                    "$forum->title<br><small>$forum->description</small>",
                    '',
                    time_diff_str($forum->timestamp, time()),
                    "<input type=checkbox name=unsub_forum_$id>"
                ]);
            }
        }
        if (count($subs)) {
            end_table();
            echo '
                <input type=submit name=submit value="Unsubscribe from selected items">
                </form>
            ';
        }
    }

    page_tail();
    BoincForumLogging::cleanup();
}

function do_unsubscribe($user) {
    $subs = BoincSubscription::enum("userid=$user->id");
    foreach ($subs as $sub) {
        if ($sub->threadid > 0) {
            $x = "unsub_thread_$sub->threadid";
        } else {
            $x = sprintf('unsub_forum_%d', -$sub->threadid);
        }
        if (post_str($x, true)) {
            BoincSubscription::delete($user->id, $sub->threadid);
        }
    }
    header('Location: forum_index.php');
}

$user = get_logged_in_user(false);
BoincForumPrefs::lookup($user);

if (DISABLE_FORUMS && !is_admin($user)) {
    error_page("Forums are disabled");
}

// Process request to mark all posts as read
//
if ((get_int("read", true) == 1)) {
    if ($user) {
        check_tokens($user->authenticator);
        $now = time();
        $user->prefs->update("mark_as_read_timestamp=$now");
        Header("Location: ".get_str("return", true));
    }
}

$submit = post_str('submit', true);
if ($submit) {
    do_unsubscribe($user);
} else {
    main($user);
}
?>
