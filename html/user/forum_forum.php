<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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

// forum page: display 'New thread' button and list of threads

require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');
require_once('../inc/pm.inc');

// show a forum.
// $user is null if not logged in
//
function forum_page($forum, $user, $msg=null) {
    global $forum_sort_styles;

    if (DISABLE_FORUMS) {
        if (!$user || !is_admin($user)) {
            error_page("Forums are disabled");
        }
    }
    $sort_style = get_int("sort", true);
    $start = get_int("start", true);
    if (!$start) $start = 0;

    $subs = [];
    if ($user) {
        BoincForumPrefs::lookup($user);
        $subs = BoincSubscription::enum("userid=$user->id");
    }

    if (!is_forum_visible_to_user($forum, $user)) {
        if ($user) {
            remove_subscriptions_forum($user->id, $forum->id);
        }
        error_page(tra("Not visible to you"));
    }

    if (!$sort_style) {
        // get the sort style either from the logged in user or a cookie
        if ($user){
            $sort_style = $user->prefs->forum_sorting;
        } else {
            list($sort_style, $thread_style) = parse_forum_cookie();
        }
    } else {
        // set the sort style
        if ($user){
            $user->prefs->forum_sorting = $sort_style;
            $user->prefs->update("forum_sorting=$sort_style");
        } else {
            list($old_style, $thread_style) = parse_forum_cookie();
            send_cookie(
                'sorting', implode("|", array($sort_style, $thread_style)), true
            );
        }
    }

    switch ($forum->parent_type) {
    case 0:
        $category = BoincCategory::lookup_id($forum->category);
        page_head(sprintf("%s '%s'", tra("Forum"), $forum->title));
        if ($msg) echo "<p>$msg</p>\n";
        show_forum_header($user);
        echo forum_title($category, $forum, NULL);
        break;
    case 1:
        $team = BoincTeam::lookup_id($forum->category);
        page_head(tra("Team message board for %1", $team->name));
        if ($msg) echo "<p>$msg</p>\n";
        show_forum_header($user);
        echo team_forum_title($forum);
        break;
    }

    echo '
        <p>
        <form action="forum_forum.php" method="get" class="form-inline">
        <input type="hidden" name="id" value="'.$forum->id.'">
        <table width="100%" cellspacing="0" cellpadding="0">
        <tr valign="top">
        <td colspan=2>
    ';

    switch (user_can_create_thread($user, $forum)) {
    case 'yes':
        show_button(
            "forum_post.php?id=$forum->id", tra("New thread"),
            tra("Add a new thread to this forum")
        );
        break;
    case 'login':
        echo "To add a thread, you must <a href=login_form.php>log in</a>.";
        break;
    }

    if ($user) {
        if (is_subscribed(-$forum->id, $subs)) {
            BoincNotify::delete_aux(sprintf(
                'userid=%d and type=%d and opaque=%d',
                $user->id,
                NOTIFY_SUBSCRIBED_FORUM,
                $forum->id
            ));
            show_button_small(
                "forum_forum.php?id=$forum->id&action=unsubscribe",
                'Unsubscribe',
                'Unsubscribe from this forum'
            );
        } else {
            show_button_small(
                "forum_forum.php?id=$forum->id&action=subscribe",
                'Subscribe',
                'Click to get notified when there are new threads in this forum'
            );
        }
    }

    echo '</td>
        <td valign=top align="right">
        <div class="form-group">
    ';
    echo select_from_array("sort", $forum_sort_styles, $sort_style);
    echo sprintf('
        <input class="btn btn-sm" %s type="submit" value="Sort">
        </div>
        </td>
        </tr>
        </table>
        </form>
        <p></p> ',
        button_style()
    );

    show_forum_threads($forum, $start, $sort_style, $user, $subs);

    echo "
        <p>".
        tra("This message board is available as an %1 RSS feed %2", "<a href=forum_rss.php?forumid=$forum->id&setup=1>", "<img src=img/feed_logo.png></a>");

    page_tail();
}

// Show the threads for the given forum
// starting from $start,
// using the given $sort_style (as defined in forum.php)
// $user is logged-in user, or null
// If $user is not null, $subs is list of their subscriptions
//
function show_forum_threads($forum, $start, $sort_style, $user, $subs) {
    $page_nav = page_links(
        "forum_forum.php?id=$forum->id&amp;sort=$sort_style",
        $forum->threads,
        THREADS_PER_PAGE,
        $start
    );
    echo $page_nav;
    start_table('table-striped');
    row_heading_array(
        array(
            "",
            tra("Threads"),
            tra("Posts"),
            tra("Author"),
            tra("Views"),
            "<nobr>".tra("Last post")."</nobr>"
        ),
        array("", "width=35%", "", "", "", "")

    );

    $sticky_first = !$user || !$user->prefs->ignore_sticky_posts;

    // Show hidden threads if logged in user is a moderator
    //
    $show_hidden = is_moderator($user, $forum);
    $threads = get_forum_threads(
        $forum->id, $start, THREADS_PER_PAGE,
        $sort_style, $show_hidden, $sticky_first
    );

    // Run through the list of threads, displaying each of them
    //
    foreach ($threads as $thread) {
        $owner = BoincUser::lookup_id($thread->owner);
        if (!$owner) continue;
        if (!$show_hidden && is_banished($owner)) continue;
        $unread = thread_is_unread($user, $thread);

        //if ($thread->status==1){
            // This is an answered helpdesk thread
        if ($user && is_subscribed($thread->id, $subs)) {
            echo '<tr class="bg-info">';
        } else {
            // Just a standard thread.
            echo '<tr>';
        }

        echo "<td width=\"1%\"><nobr>";
        if ($thread->hidden) {
            show_image(IMAGE_HIDDEN, tra("This thread is hidden"), tra("hidden"));
        } else if ($unread) {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_STICKY_LOCKED, tra("This thread is sticky and locked, and you haven't read it yet"), tra("sticky/locked/unread"));
                } else {
                    show_image(NEW_IMAGE_STICKY, tra("This thread is sticky and you haven't read it yet"), tra("sticky/unread"));
                }
            } else {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_LOCKED, tra("You haven't read this thread yet, and it's locked"), tra("unread/locked"));
                } else {
                    show_image(NEW_IMAGE, tra("You haven't read this thread yet"), tra("unread"));
                }
            }
        } else {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(IMAGE_STICKY_LOCKED, tra("This thread is sticky and locked"), tra("sticky/locked"));
                } else {
                    show_image(IMAGE_STICKY, tra("This thread is sticky"), tra("sticky"));
                }
            } else {
                if ($thread->locked) {
                    show_image(IMAGE_LOCKED, tra("This thread is locked"), tra("locked"));
                } else {
                    show_image(IMAGE_POST, tra("You read this thread"), tra("read"));
                }
            }
        }
        echo "</nobr></td>";

        $title = cleanup_title($thread->title);
        //$titlelength = 9999;
        //if (strlen($title) > $titlelength) {
        //    $title = substr($title, 0, $titlelength)."...";
        //}
        echo "<td><a href=\"forum_thread.php?id=$thread->id\">$title</a><br></td>";

        echo '
            <td>'.($thread->replies+1).'</td>
            <td>'.user_links($owner, BADGE_HEIGHT_SMALL).'</td>
            <td>'.$thread->views.'</td>
            <td>'.time_diff_str($thread->timestamp, time()).'</td>
            </tr>
        ';
        flush();
    }
    end_table();
    echo "<br>$page_nav";    // show page links
}

$id = get_int("id");
$forum = BoincForum::lookup_id($id);
if (!$forum) error_page("forum ID not found");
$user = get_logged_in_user(false);
$action = get_str('action', true);

if ($action == 'subscribe') {
    $user = get_logged_in_user();
    BoincSubscription::replace($user->id, -$id);
    forum_page($forum, $user, 'You are now subscribed to this forum.');
} else if ($action == 'unsubscribe') {
    $user = get_logged_in_user();
    BoincSubscription::delete($user->id, -$id);
    forum_page($forum, $user, 'You are now unsubscribed from this forum.');
} else {
    forum_page($forum, $user);
}

?>
