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

// display the threads in a forum.

require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');
require_once('../inc/pm.inc');

$id = get_int("id");
$sort_style = get_int("sort", true);
$start = get_int("start", true);
if (!$start) $start = 0;

$forum = BoincForum::lookup_id($id);
if (!$forum) error_page("no forum");
$user = get_logged_in_user(false);

if (!is_forum_visible_to_user($forum, $user)) {
    if ($user) {
        remove_subscriptions_forum($user->id, $id);
    }
    error_page("Not visible");
}

BoincForumPrefs::lookup($user);

if (!$sort_style) {
    // get the sort style either from the logged in user or a cookie
    if ($user){
        $sort_style = $user->prefs->forum_sorting;
    } else {
        if (isset($_COOKIE['sorting'])) {
            list($sort_style,$thread_style)=explode("|",$_COOKIE['sorting']);
        }
    }
} else {
    // set the sort style
    if ($user){
        $user->prefs->forum_sorting = $sort_style;
        $user->prefs->update("forum_sorting=$sort_style");
    } else {
        list($old_style,$thread_style)=explode("|",$_COOKIE['sorting']);
        send_cookie('sorting', implode("|",array($sort_style,$thread_style)), true);
    }
}


switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category); 
    if ($category->is_helpdesk){
        page_head(tra("Questions and Answers").' : '.$forum->title);
        echo '<link href="forum_help_desk.php" rel="up" title="Forum Index">';
    } else {
        page_head(tra("Message boards").' : '.$forum->title);
        echo '<link href="forum_index.php" rel="up" title="Forum Index">';
    }
    show_forum_header($user);
    show_forum_title($category, $forum, NULL);
    break;
case 1:
    $team = BoincTeam::lookup_id($forum->category); 
    page_head("Team message board for <a href=team_display.php?teamid=$team->id>$team->name</a>");
    show_forum_header($user);
    show_team_forum_title($forum);
    break;
}

echo '
    <p>
    <form action="forum_forum.php" method="get">
    <table width="100%" cellspacing="0" cellpadding="0">
    <tr valign="top">
    <td colspan=2>
';

if (user_can_create_thread($user, $forum)) {
    show_button(
        "forum_post.php?id=$id", "New thread", "Add a new thread to this forum"
    );
}

echo "</td>
    <td valign=top align=\"right\">
    <input type=\"hidden\" name=\"id\" value=\"$forum->id\">
";
echo select_from_array("sort", $forum_sort_styles, $sort_style);
echo "<input type=\"submit\" value=\"Sort\">
    </td>
    </tr>
    </table>
    </form>
";

show_forum($forum, $start, $sort_style, $user);

echo "
    <p>
    This message board is available as an
    <a href=forum_rss.php?forumid=$forum->id&setup=1>RSS feed
    <img src=img/feed_logo.png></a>
";

page_tail();

// This function shows the threads for the given forum
// Starting from $start,
// using the given $sort_style (as defined in forum.php)
// and using the features for the logged in user in $user.
//
function show_forum($forum, $start, $sort_style, $user) {
    $gotoStr = "";
    $nav = show_page_nav($forum, $start);
    if ($nav) {
        $gotoStr = "<div align=\"right\">$nav</div><br>";
    }
    echo $gotoStr; // Display the navbar
    start_forum_table(array("", tra("Threads"), tra("Posts"), tra("Author"), tra("Views"), "<nobr>".tra("Last post")."</nobr>"));
    
    $sticky_first = !$user || !$user->prefs->ignore_sticky_posts;

    // Show hidden threads if logged in user is a moderator
    //
    $show_hidden = is_moderator($user, $forum);
    $threads = get_forum_threads(
        $forum->id, $start, THREADS_PER_PAGE,
        $sort_style, $show_hidden, $sticky_first
    );

    if ($user) {
        $subs = BoincSubscription::enum("userid=$user->id");
    }
    
    // Run through the list of threads, displaying each of them
    $n = 0; $i=0;
    foreach ($threads as $thread) {
        $owner = BoincUser::lookup_id($thread->owner);
        $unread = thread_is_unread($user, $thread);
        
        //if ($thread->status==1){
            // This is an answered helpdesk thread
        if ($user && is_subscribed($thread, $subs)) {
            echo '<tr class="row_hd'.$n.'">';
        } else {
            // Just a standard thread.
            echo '<tr class="row'.$n.'">';    
        }
        
        echo "<td width=\"1%\" align=\"right\"><nobr>";
        if ($thread->hidden) {
            echo "[hidden]";
        }
        if ($unread) {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_STICKY_LOCKED, "This thread is sticky and locked, and you haven't read it yet", "sticky/locked/unread");
                } else {
                    show_image(NEW_IMAGE_STICKY, "This thread is sticky and you haven't read it yet", "sticky/unread");
                }
            } else {
                if ($thread->locked) {
                    show_image(NEW_IMAGE_LOCKED, "You haven't read this thread yet, and it's locked", "unread/locked");
                } else {
                    show_image(NEW_IMAGE, "You haven't read this thread yet", "unread");
                }
            }
        } else {
            if ($thread->sticky) {
                if ($thread->locked) {
                    show_image(IMAGE_STICKY_LOCKED, "This thread is sticky and locked", "sticky/locked");
                } else {
                    show_image(IMAGE_STICKY, "This thread is sticky", "sticky");
                }
            } else {
                if ($thread->locked) {
                    show_image(IMAGE_LOCKED, "This thread is locked", "locked");
                }
            }
        }
        echo "</nobr></td>";

        $titlelength = 48;
        $title = cleanup_title($thread->title);
        if (strlen($title) > $titlelength) {
            $title = substr($title, 0, $titlelength)."...";
        }
        echo "<td class=\"threadline\"><a href=\"forum_thread.php?id=$thread->id\"><b>$title</b></a><br></td>";
        $n = ($n+1)%2;

        echo '
            <td>'.($thread->replies+1).'</td>
            <td>'.user_links($owner).'</td>
            <td>'.$thread->views.'</td>
            <td class=\"lastpost\">'.time_diff_str($thread->timestamp, time()).'</td>
            </tr>
        ';
        flush();
    }
    end_table();
    echo "<br>$gotoStr";    // show page links
}

?>
