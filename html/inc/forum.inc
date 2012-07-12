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

require_once("../inc/forum_db.inc");
require_once("../inc/pm.inc");
require_once("../inc/team.inc");
require_once("../inc/news.inc");
require_once("../inc/text_transform.inc");

define('THREADS_PER_PAGE', 50);

// sorting styles
define('MODIFIED_NEW', 1);
define('MODIFIED_OLD',2);
define('VIEWS_MOST',3);
define('REPLIES_MOST',4);
define('CREATE_TIME_NEW',5);
define('CREATE_TIME_OLD',6);
define('POST_SCORE',7);

// names for the above
$thread_sort_styles[CREATE_TIME_OLD] = tra("Oldest first");
$thread_sort_styles[CREATE_TIME_NEW] = tra("Newest first");
$thread_sort_styles[POST_SCORE] = tra("Highest rated posts first");

$forum_sort_styles[MODIFIED_NEW] = tra("Newest post first");
$forum_sort_styles[VIEWS_MOST] = tra("Most views first");
$forum_sort_styles[REPLIES_MOST] = tra("Most posts first");
$forum_sort_styles[CREATE_TIME_NEW] = tra("Newest first");

// values for thread.status
define('THREAD_SOLVED', 1);

define('AVATAR_WIDTH', 100);
define('AVATAR_HEIGHT',100);

define('ST_NEW_TIME', 1209600); //3600*24*14 - 14 days
define('ST_NEW', 'New member');

define('MAXIMUM_EDIT_TIME',3600);
    // allow edits of forums posts up till one hour after posting.

define('MAX_FORUM_LOGGING_TIME', 2419200); //3600*24*28 - 28 days
define('NO_CONTROLS', 0);
define('FORUM_CONTROLS', 1);
define('HELPDESK_CONTROLS', 2);
define("EXCERPT_LENGTH", "120");

define('NEW_IMAGE', 'img/unread_post.png');
define('NEW_IMAGE_STICKY', 'img/unread_sticky.png');
define('NEW_IMAGE_LOCKED', 'img/unread_locked.png');
define('NEW_IMAGE_STICKY_LOCKED', 'img/unread_sticky_locked.png');
define('IMAGE_STICKY', 'img/sticky_post.png');
define('IMAGE_LOCKED', 'img/locked_post.png');
define('IMAGE_HIDDEN', 'img/hidden.png');
define('IMAGE_STICKY_LOCKED', 'img/sticky_locked_post.png');
define('IMAGE_POST', 'img/post.png');
define('NEW_IMAGE_HEIGHT','15');
define('EMPHASIZE_IMAGE', 'img/emphasized_post.png');
define('EMPHASIZE_IMAGE_HEIGHT','15');
define('FILTER_IMAGE', 'img/filtered_post.png');
define('FILTER_IMAGE_HEIGHT','15');
define('RATE_POSITIVE_IMAGE', 'img/rate_positive.png');
define('RATE_POSITIVE_IMAGE_HEIGHT','9');
define('RATE_NEGATIVE_IMAGE', 'img/rate_negative.png');
define('RATE_NEGATIVE_IMAGE_HEIGHT','9');
define('REPORT_POST_IMAGE', 'img/report_post.png');
define('REPORT_POST_IMAGE_HEIGHT','9');

define ('SOLUTION', 'This answered my question');
define ('SUFFERER', 'I also have this question');
define ('OFF_TOPIC', 'Off-topic');

define ('DEFAULT_LOW_RATING_THRESHOLD', -25);
define ('DEFAULT_HIGH_RATING_THRESHOLD', 5);

// A list of what kind of special users exist
define('S_MODERATOR', 0);
define('S_ADMIN', 1);
define('S_DEV', 2);
define('S_TESTER', 3);
define('S_VOLUNTEER', 4);
define('S_VOLUNTEER_TESTER', 5);
define('S_SCIENTIST', 6);

$special_user_bitfield[S_MODERATOR]="Volunteer moderator";
$special_user_bitfield[S_ADMIN]="Project administrator";
$special_user_bitfield[S_DEV]="Project developer";
$special_user_bitfield[S_TESTER]="Project tester";
$special_user_bitfield[S_VOLUNTEER]="Volunteer developer";
$special_user_bitfield[S_VOLUNTEER_TESTER]="Volunteer tester";
$special_user_bitfield[S_SCIENTIST]="Project scientist";

// show a banner with search form on left and PM info on right
//
function show_forum_header($user) {
    echo "<form action=\"forum_search_action.php\" method=\"POST\">
    ";
    echo "<table cellpadding=\"5\" width=\"100%\" class=\"forum_header\">";
    echo "<tr>
    ";
    
    // Search
    echo "
        <td>
        <input type=\"hidden\" name=\"search_max_time\" value=\"30\">
        <input type=\"hidden\" name=\"search_forum\" value=\"-1\">
        <input type=\"hidden\" name=\"search_sort\" value=\"5\">
        <input type=\"text\" name=\"search_keywords\">
        <input class=\"btn\" title=\"".tra("Search for words in forum messages")."\" type=\"submit\" value=\"".tra("Search forums")."\"><br>
        <span class=\"smalltext\"><a href=\"forum_search.php\">".tra("Advanced search")."</a></span>
        </td>
    ";
    
    if ($user) {
        echo "<td align=\"right\">\n";
        echo "<p>".tra("Private messages").": ", pm_notification($user);
        echo "</td>\n";
    }
    echo "</tr>
    ";
    end_table();
    echo "</form>
    ";
}

// Output the forum/thread title.
//
function show_forum_title($category, $forum, $thread, $link_thread=false) {
    if ($category) {
        $is_helpdesk = $category->is_helpdesk;
    } else {
        $is_helpdesk = false;
    }

    $where = $is_helpdesk?tra("Questions and Answers"):tra("Message boards");
    $top_url = $is_helpdesk?"forum_help_desk.php":"forum_index.php";

    if (!$forum && !$thread) {
        echo "<span class=\"title\">$where</span>\n";

    } else if ($forum && !$thread) {
        echo "<span class=title>";
        echo "<a href=\"$top_url\">$where</a> : ";
        echo $forum->title;
        echo "</span>";
    } else if ($forum && $thread) {
        echo "<span class=title>
            <a href=\"$top_url\">$where</a> : 
            <a href=\"forum_forum.php?id=".$forum->id."\">", $forum->title, "</a> : 
        ";
        if ($link_thread) {
            echo "<a href=forum_thread.php?id=$thread->id>";
        }
        echo cleanup_title($thread->title);
        if ($link_thread) {
            echo "</a>";
        }
        echo "</span>";
    } else {
        echo "Invalid thread ID";
    }
}

function show_team_forum_title($forum, $thread=null, $link_thread=false) {
    $team = BoincTeam::lookup_id($forum->category);
    echo "<span class=title>
        <a href=\"forum_index.php\">".tra("Message boards")."</a> :
    ";
    if ($thread) {
        echo "
            <a href=team_forum.php?teamid=$team->id>".tra("%1 message board", $team->name)."</a>
        ";
        if ($link_thread) {
            echo " : <a href=forum_thread.php?id=$thread->id>$thread->title</a>";
        } else {
            echo " : $thread->title";
        }
    } else {
        echo tra("%1 message board", $team->name);
    }
    echo "</span>";
}

function start_forum_table($headings, $extra=null) {
    $span = null;
    
    start_table($extra." width=\"100%\"");
    echo "<tr>";

    for ($i=0; $i<count($headings); $i++) {
        if (is_array($headings[$i])){
            $title = $headings[$i][0];
            $class = $headings[$i][1]?$headings[$i][1]:"heading";
            if (isset($headings[$i][2])) {
                $span = " colspan=\"".$headings[$i][2]."\" ";
            }
        } else {
            $title = $headings[$i];
            $class = "heading";
            $span="";
        }
        echo "<th class=$class$span>$title</th>\n";
    }
    echo "</tr>\n";
}

function page_link($url, $page_num, $items_per_page, $text) {
    return " <a href=\"$url&amp;start=" . $page_num*$items_per_page . "\">$text</a> ";
}

// return a string for navigating pages
//
function page_links($url, $nitems, $items_per_page, $start){
    // How many pages to potentially show before and after this one:
    $preshow = 3;
    $postshow = 3;

    $x = "";
    
    if ($nitems <= $items_per_page) return "";
    $npages = ceil($nitems / $items_per_page);
    $curpage = ceil($start / $items_per_page);

    // If this is not the first page, display "previous"
    //
    if ($curpage > 0){
        $x .= page_link(
            $url, $curpage-1, $items_per_page,
            tra("Previous")." &middot; "
        );
    }

    if ($curpage - $preshow > 0) {
        $x .= page_link($url, 0, $items_per_page, "1");
        if ($curpage - $preshow > 1) {
            $x .= " . . . ";
        } else {
            $x .= " &middot; ";
        }
    }
    // Display a list of pages surrounding this one
    //
    for ($i=$curpage-$preshow; $i<=$curpage+$postshow; $i++){
        $page_str = (string)($i+1);
        if ($i < 0) continue;
        if ($i >= $npages) break;

        if ($i == $curpage) {
            $x .= "<b>$page_str</b>";
        } else {
            $x .= page_link($url, $i, $items_per_page, $page_str);
        }
        if ($i == $npages-1) break;
        if ($i == $curpage+$postshow) break;
        $x .= " &middot; ";
    }

    if ($curpage + $postshow < $npages-1) {
        $x .= " . . . ";
        $x .= page_link($url, $npages-1, $items_per_page, $npages);
    }
    // If there is a next page
    //
    if ($curpage < $npages-1){
        $x .= page_link(
            $url, $curpage+1, $items_per_page,
            " &middot; ".tra("Next")
        );
    }
    $x .= "\n";
    return $x;
}

function thread_is_unread($user, $thread) {
    if (!$user) return false;
    if ($thread->timestamp <= $user->prefs->mark_as_read_timestamp) return false;
    $log = BoincForumLogging::lookup($user->id, $thread->id);
    if ($log && ($thread->timestamp <= $log->timestamp)) return false;
    return true;
}

//  Process a user-supplied title to remove HTML stuff
//
function cleanup_title($title) {
    $x = sanitize_tags(bb2html($title));
    $x = trim($x);
    if (strlen($x)==0) return "(no title)";
    else return $x;
}

function can_reply($thread, $forum, $user) {
    if ($thread->locked) {
        if (!is_moderator($user, $forum)) return false;
    }
    return true;
}

// Show the posts in a thread for a user.
// If $start is null, enforce jump-to-first-unread
//
function show_posts(
    $thread, $forum, $start, $sort_style, $filter, $logged_in_user,
    $show_controls=true
) {
    $n = 1;
    
    if ($show_controls) {
        $controls = FORUM_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }

    $num_to_show = 20;
    if ($logged_in_user && $logged_in_user->prefs->display_wrap_postcount > 0) {
        $num_to_show = $logged_in_user->prefs->display_wrap_postcount;
    }

    // let moderators see all posts, including hidden ones
    //
    if (is_moderator($logged_in_user, $forum)) {
        $show_hidden = true;
    } else {
        $show_hidden = false;
    }

    $posts = get_thread_posts($thread->id, $sort_style, $show_hidden);

    $latest_viewed = 0;
    $forum_log = null;
    if ($logged_in_user) {
        $forum_log = BoincForumLogging::lookup($logged_in_user->id, $thread->id);
        if ($forum_log) {
            $latest_viewed = $forum_log->timestamp;
        }
    }

    // jump to first unread post if needed
    //
    $first_unread_post = null;
    if ($start === null) {
        if ($latest_viewed && $logged_in_user->prefs->jump_to_unread) {
            $i = 0;
            $ibest = 0;
            foreach ($posts as $post) {
                if ($post->timestamp > $latest_viewed) {
                    if (!$first_unread_post || ($post->timestamp < $first_unread_post->timestamp)) {
                        $first_unread_post = $post;
                        $ibest = $i;
                    }
                }
                $i++;
            }
            if ($first_unread_post) {
                $start = $ibest - ($ibest % $num_to_show);
            } else {
                $start = 0;
            }
        } else {
            $start = 0;
        }
    }

    $page_nav = page_links(
        "forum_thread.php?id=$thread->id&sort_style=$sort_style",
        sizeof($posts),
        $num_to_show,
        $start
    );

    echo $page_nav;

    $num_shown = 0;
    $num_skipped = 0;
    $headings = array(array(tra("Author"),"authorcol"), array(tra("Message"),""));
    start_forum_table($headings, "id=\"thread\" cellspacing=0");

    $latest_post_shown = 0;
    $first_unread_post = null;
    foreach ($posts as $post) {
        if ($num_skipped < $start) {
            $num_skipped++;
            continue;
        }
        if ($num_shown == $num_to_show) {
            break;
        }
        show_post(
            $post, $thread, $forum, $logged_in_user, $latest_viewed, $n,
            $controls, $filter
        );
        $n = ($n+1)%2;
        
        if ($post->timestamp > $latest_post_shown) {
            $latest_post_shown = $post->timestamp;
        }
        $num_shown++;
    }
    end_table();
    echo $page_nav;

    if ($first_unread_post) {
        echo "<script>function jumpToUnread(){location.href='#".$first_unread_post->id."';}</script>";
    } else {
        echo "<script>function jumpToUnread(){};</script>";
    }

    if (!$forum_log || $latest_post_shown > $forum_log->timestamp) {
        BoincForumLogging::replace(
            $logged_in_user->id, $thread->id, $latest_post_shown
        );
    }
}

function get_ignored_list($user) {
    return explode("|", $user->prefs->ignorelist);
}

function add_ignored_user($user, $other_user) {
    $list = explode("|", $user->prefs->ignorelist);
    foreach ($list as $key=>$userid) {
        if ($userid == $other_user->id) {
            return true;
        }
    }
    $list[] = $other_user->id;
    $x = implode("|", array_values($list));
    return $user->prefs->update("ignorelist='$x'");
}

function remove_ignored_user($user, $other_user) {
    $list = explode("|", $user->prefs->ignorelist);
    foreach ($list as $key=>$userid) {
        if ($userid == $other_user->id) {
            unset($list[$key]);
        }
    }
    $x = implode("|", array_values($list));
    return $user->prefs->update("ignorelist='$x'");
}

function is_ignoring($user, $other_user) {
    $list = explode("|", $user->prefs->ignorelist);
    return in_array($other_user->id, $list);
}

// Display an individual post
//
function show_post(
    $post, $thread, $forum, $logged_in_user, $latest_viewed, $n,
    $controls=FORUM_CONTROLS, $filter=true
) {
    global $country_to_iso3166_2;

    $user = BoincUser::lookup_id($post->user);
    BoincForumPrefs::lookup($user);
    if (is_banished($user) && !is_moderator($logged_in_user, $forum)) {
        return;
    }

    // If the user no longer exists, skip the post
    //
    if (!$user){
        return;
    }

    $config = get_config();
    $no_forum_rating = parse_bool($config, "no_forum_rating");

    $tokens = "";
    $options = get_output_options($logged_in_user);

    // check whether the poster is on the list of people to ignore
    //
    $ignore_poster = false;
    if ($logged_in_user){
        $tokens = url_tokens($logged_in_user->authenticator);
        if (is_ignoring($logged_in_user, $user)){
            $ignore_poster = true;
        }
    }

    // The creator can edit the post, but only in a specified amount of time
    // (exception: a moderator can edit his/her posts at any time)
    //
    $can_edit = false;
    if ($logged_in_user) {
        if ($user->id == $logged_in_user->id) {
            if (is_moderator($logged_in_user, $forum)) {
                $can_edit = true;
            } else if (can_reply($thread, $forum, $logged_in_user)) {
                $time_limit = $post->timestamp+MAXIMUM_EDIT_TIME;
                $can_edit = time()<$time_limit;
            } else {
                $can_edit = false;
            }
        }
    }


    // Print the special user lines, if any
    //
    global $special_user_bitfield;
    $fstatus="";
    $keys = array_keys($special_user_bitfield);
    $is_posted_by_special = false;
    for ($i=0; $i<sizeof($special_user_bitfield);$i++) {
        if ($user->prefs && $user->prefs->privilege($keys[$i])) {
            $fstatus.=$special_user_bitfield[$keys[$i]]."<br>";
            $is_posted_by_special = true;
        }
    }
    
    // Highlight special users if set in prefs;
    //
    if ($logged_in_user && $logged_in_user->prefs){
        $highlight = $logged_in_user->prefs->highlight_special && $is_posted_by_special;
    } else {
        $highlight = $is_posted_by_special;
    }
    echo "
        <tr>
        <td class=\"leftcol ".($highlight?"highlighted_":"")."row$n\" rowspan=\"3\">
        <a name=\"$post->id\"></a>
        <div class=\"authorcol\">
    ";

    echo user_links($user);
    echo "<br>";
    if ($user->create_time > time()-ST_NEW_TIME) $fstatus.=ST_NEW."<br>";
    if ($fstatus) echo "<font size=\"-2\">$fstatus</font>";

    echo "<span class=\"authorinfo\">";
    if (!$filter || !$ignore_poster){
        if ($user->prefs && $user->prefs->avatar!="" && (!$logged_in_user || ($logged_in_user->prefs->hide_avatars==false))) {
            echo "<img class=authorinfo width=\"".AVATAR_WIDTH."\" height=\"".AVATAR_HEIGHT."\" src=\"".$user->prefs->avatar."\" alt=\"Avatar\"><br>";
        }
    }
    
    $url = "pm.php?action=new&amp;userid=".$user->id;
    $name = $user->name;
    show_button($url, tra("Send&nbsp;message"), tra("Send %1 a private message",$name));
    echo "<br>".tra("Joined: %1", gmdate('j M y', $user->create_time)), "<br>";

    if (!isset($user->nposts)) {
        $user->nposts = BoincPost::count("user=$user->id");
    }
    
    if (function_exists('project_forum_user_info')){
        project_forum_user_info($user);
    } else {
        echo tra("Posts: %1", $user->nposts)."<br>";
        // circumvent various forms of identity spoofing
        // by displaying the  user id of the poster.
        //
        //echo "ID: ".$user->id."<br>";
        if (!no_computing()) {
            echo tra("Credit: %1", number_format($user->total_credit)) ."<br>";
            echo tra("RAC: %1",    number_format($user->expavg_credit))."<br>";
        }
        // to use this feature:
        // - get flags from http://www.famfamfam.com/lab/icons/flags/famfamfam_flag_icons.zip
        // - put the .png's in html/user/flags/
        // - put define(COUNTRY_FLAGS, 1) in your html/project/project.inc
        //
        if (defined("COUNTRY_FLAGS")) {
            if (array_key_exists($user->country, $country_to_iso3166_2)) {
                $code = $country_to_iso3166_2[$user->country];
                echo "<img class=flag alt=\"$user->country\" title=\"$user->country\" src=flags/$code.png><br>\n";
            }
        }
    }
    echo "</span></div></td>";

    echo "<td class=\"postheader\">";
    if ($controls == FORUM_CONTROLS) {
        echo "<form action=\"forum_rate.php?post=", $post->id, "\" method=\"post\">";
    }

    if ($logged_in_user && $post->timestamp>$latest_viewed){
        show_image(NEW_IMAGE, tra("You haven't read this message yet"), tra("Unread"), NEW_IMAGE_HEIGHT);
    }

    echo " <a href=\"forum_thread.php?id=".$thread->id."&amp;nowrap=true#$post->id\">".tra("Message %1", $post->id)."</a> - ";
    if ($post->hidden) echo "<font color=red>[".tra("hidden")."] </font>";
    echo tra("Posted: %1", pretty_time_str($post->timestamp)), " ";

    if ($post->parent_post) {
        echo tra(" - in response to ")."<a href=\"forum_thread.php?id=".$thread->id."&amp;nowrap=true#".$post->parent_post."\">".tra("Message %1", $post->parent_post)."</a>.";
    }
    if ($can_edit && $controls != NO_CONTROLS) {
        show_button("forum_edit.php?id=".$post->id."$tokens", tra("Edit"), tra("Edit this message"));
    }
    if (is_moderator($logged_in_user, $forum)) {
        show_post_moderation_links($config, $logged_in_user, $post, $forum, $tokens);
    }
    if ($post->modified) {
        echo "<br>".tra("Last modified: %1", pretty_time_Str($post->modified));
    }
    if ($ignore_poster && $filter){
        echo "<br>".tra("This post is not shown because the sender is on your 'ignore' list.  Click %1here%2 to view this post","<a href=\"?id=".$thread->id."&amp;filter=false#".$post->id."\">","</a>");
    }
    if ($controls == FORUM_CONTROLS) {
        echo "</form>\n";
    }
    echo "</td>
        </tr>
        <tr class=\"".($highlight?"highlighted_":"")."row$n\">
        <td class=\"postbody\">
    ";

    if (!$filter || !$ignore_poster){
        $posttext = $post->content;

        // If the creator of this post has a signature and
        // wants it to be shown for this post AND the logged in
        // user has signatures enabled: show it
        //
        if ($post->signature && (!$logged_in_user || !$logged_in_user->prefs->hide_signatures)){
            $posttext.="\n____________\n".$user->prefs->signature;
        }

        $posttext = output_transform($posttext, $options);
        
        echo "<p>", $posttext, "</p>";
        echo "</td></tr><tr><td class=\"postfooter\">ID: <i>", $post->id;
        if ($no_forum_rating) {
            echo " | <a href=\"forum_report_post.php?post=".$post->id."\">";
            show_image(REPORT_POST_IMAGE, tra("Report this post as offensive"), tra("Report as offensive"), REPORT_POST_IMAGE_HEIGHT);
            echo "</a>";
        } else {
            $rating = $post->rating();
            echo " | ".tra("Rating: %1", $rating)."</i> | ".tra("rate: ")."
                <a href=\"forum_rate.php?post=".$post->id."&amp;choice=p$tokens\">
            ";
            show_image(RATE_POSITIVE_IMAGE, tra("Click if you like this message"), tra("Rate +"), RATE_POSITIVE_IMAGE_HEIGHT);
            echo "</a> / <a href=\"forum_rate.php?post=".$post->id."&amp;choice=n$tokens\">";
            show_image(RATE_NEGATIVE_IMAGE, tra("Click if you don't like this message"), tra("Rate -"), RATE_NEGATIVE_IMAGE_HEIGHT);
            echo "</a> <a href=\"forum_report_post.php?post=".$post->id."\">";
            show_image(REPORT_POST_IMAGE, tra("Report this post as offensive"), tra("Report as offensive"), REPORT_POST_IMAGE_HEIGHT);
            echo "</a>";
        }
        if (($controls == FORUM_CONTROLS) && (can_reply($thread, $forum, $logged_in_user))) {
            echo "&nbsp;&nbsp;&nbsp;&nbsp;";
            $url = "forum_reply.php?thread=" . $thread->id . "&amp;post=" . $post->id . "&amp;no_quote=1#input";
            show_button($url, tra("Reply"), tra("Post a reply to this message"));
            $url = "forum_reply.php?thread=" . $thread->id . "&amp;post=" . $post->id . "#input";
            show_button($url, tra("Quote"), tra("Post a reply by quoting this message"));
        }
        echo "</td></tr>";
    } else {
        echo "</td></tr><tr><td class=\"postfooter\">";
    }
    echo "<tr class=\"postseparator\"><td colspan=2></td></tr>";
}

// Show a post and its context (e.g. for search results, user posts)
//
function show_post_and_context($post, $thread, $forum, $options, $n) {
    $thread = BoincThread::lookup_id($post->thread);
    $forum = BoincForum::lookup_id($thread->forum);

    $content = output_transform($post->content, $options);
    $when = time_diff_str($post->timestamp, time());
    $user = lookup_user_id($post->user);
    $title = cleanup_title($thread->title);
    $m = $n%2;
    if ($post->hidden) {
        $deleted = "<br><font color=red>[".tra("Hidden by a moderator")."]</font>";
    } else {
        $deleted = "";
    }
    echo "
        <tr class=row$m>
        <td>
            $n)
    ";
    switch ($forum->parent_type) {
    case 0:
        $category = BoincCategory::lookup_id($forum->category);
        show_forum_title($category, $forum, $thread, true);
        break;
    case 1:
        show_team_forum_title($forum);
        break;
    }
    echo "
        (<a href=\"forum_thread.php?id=".$thread->id."&amp;nowrap=true#".$post->id."\">".tra("Message %1", $post->id)."</a>)
        <br>
        ".tra("Posted %1 by %2", $when, user_links($user))." $deleted
        <hr>
        $content
        </td></tr>
    ";
}

function is_banished($user) {
    if (isset($user->prefs)) {
        return ($user->prefs->banished_until > time());
    } else {
        return false;
    }
}

function check_banished($user) {
    if (is_banished($user)) {
        error_page(
            tra("You may not post or rate messages until %1", gmdate('M j, Y', $user->prefs->banished_until))
        );
    }
}

function post_rules() {
    if (function_exists("project_forum_post_rules")) {
      $project_rules=project_forum_post_rules();
    } else {
      $project_rules="";
    }
    return tra("
        <ul>
        <li> Posts must be 'kid friendly': they may not contain
            content that is obscene, hate-related,
            sexually explicit or suggestive.
        <li> No commercial advertisements.
        <li> No links to web sites involving sexual content,
            gambling, or intolerance of others.
        <li> No messages intended to annoy or antagonize other people,
            or to hijack a thread.
        <li> No messages that are deliberately hostile or insulting.
        <li> No abusive comments involving race, religion,
            nationality, gender, class or sexuality.
        ").$project_rules."
        </ul>
    ";
}

function post_warning() {
    return "<br><br>
        <table><tr><td align=left>
        <font size=-2>
        ".tra("Rules:").post_rules()."
        <a href=moderation.php>".tra("More info")."</a>
        </font>
        </td></tr></table>
    ";
}

function notify_subscriber($thread, $user) {
    BoincForumPrefs::lookup($user);
    if ($user->prefs->pm_notification == 1) {
        send_reply_notification_email($thread, $user);
    }
    $now = time();
    $type = NOTIFY_SUBSCRIBED_POST;
    BoincNotify::replace("userid=$user->id, create_time=$now, type=$type, opaque=$thread->id");
}

// Various functions for adding/hiding/unhiding stuff.
// These take care of counts and timestamps.
// Don't do these things directly - use these functions
//
function create_post($content, $parent_id, $user, $forum, $thread, $signature) {
    $content = substr($content, 0, 64000);
    $content = mysql_real_escape_string($content);
    $now = time();
    $sig = $signature?1:0;
    $id = BoincPost::insert("(thread, user, timestamp, content, parent_post, signature) values ($thread->id, $user->id, $now, '$content', $parent_id, $sig)");
    if (!$id) return null;

    // notify subscribed users
    //
    $subs = BoincSubscription::enum("threadid=$thread->id");
    foreach ($subs as $sub) {
        if ($user->id == $sub->userid) continue;
        $user2 = BoincUser::lookup_id($sub->userid);
        if ($user2) {
            notify_subscriber($thread, $user2);
        }
    }
    $user->prefs->update("posts=posts+1");
    $thread->update("replies=replies+1, timestamp=$now");
    $forum->update("posts=posts+1, timestamp=$now");
}

// call this when hide or delete a post;
// it sets timestamp to time of last non-hidden post
//
function update_thread_timestamp($thread) {
    $posts = BoincPost::enum("thread=$thread->id and hidden=0 order by timestamp desc limit 1");
    if (count($posts)>0) {
        $post = $posts[0];
        $thread->update("timestamp=$post->timestamp");
    }
}

function update_forum_timestamp($forum) {
    $threads = BoincThread::enum("forum=$forum->id and hidden=0 order by timestamp desc limit 1");
    if (count($threads)>0) {
        $thread = $threads[0];
        $forum->update("timestamp=$thread->timestamp");
    }
}

function create_thread($title, $content, $user, $forum, $signature, $export) {
    $title = trim($title);
    $title = sanitize_tags($title);
    $title = mysql_real_escape_string($title);
    $now = time();
    $status = 0;
    if (is_news_forum($forum) && !$export) {
        $status = 1;
    }
    $id  = BoincThread::insert("(forum, owner, title, create_time, timestamp, replies, status) values ($forum->id, $user->id, '$title', $now, $now, -1, $status)");
    if (!$id) return null;
    $thread = BoincThread::lookup_id($id);
    create_post($content, 0, $user, $forum, $thread, $signature);
    $forum->update("threads=threads+1");
    return $thread;
}

function hide_post($post, $thread, $forum) {
    $ret = $post->update("hidden=1");
    if (!$ret) return $ret;
    $thread->update("replies=replies-1");
    $forum->update("posts=posts-1");
    update_thread_timestamp($thread);
    update_forum_timestamp($forum);
    return true;
}

function unhide_post($post, $thread, $forum) {
    $ret = $post->update("hidden=0");
    if (!$ret) return $ret;
    $thread->update("replies=replies+1");
    $forum->update("posts=posts+1");
    update_thread_timestamp($thread);
    update_forum_timestamp($forum);
    return true;
}

function delete_post($post, $thread, $forum) {
    $post->delete();
    if (!$post->hidden) {
        $thread->update("replies=replies-1");
        $forum->update("posts=posts-1");
    }
    $count = BoincPost::count("thread=$thread->id");
    if ($count == 0) {
        $forum->update("threads=threads-1");
        $thread->delete();
    } else {
        update_thread_timestamp($thread);
    }
}

// delete all forum records related to user
//
function forum_delete_user($user) {
    $pp = BoincPost::enum("user=$user->id");
    foreach ($pp as $p) {
        $t = BoincThread::lookup_id($p->thread);
        $f = BoincForum::lookup_id($t->forum);
        if ($t && $f) {
            delete_post($p, $t, $f);
        }
    }
    $ss = BoincSubscription::enum("userid=$user->id");
    foreach ($ss as $s) {
        BoincSubscription::delete($s->userid, $s->threadid);
    }
    $p = BoincForumPrefs::lookup_userid($user->id);
    $p->delete();
}

function move_post($post, $old_thread, $old_forum, $new_thread, $new_forum) {
    $post->update("thread=$new_thread->id");
    $old_thread->update("replies=replies-1");
    $new_thread->update("replies=replies+1");
    $old_forum->update("posts=posts-1");
    $new_forum->update("posts=posts+1");
    update_thread_timestamp($old_thread);
    update_thread_timestamp($new_thread);
    update_forum_timestamp($old_forum);
    update_forum_timestamp($new_forum);
    return true;
}

function hide_thread($thread, $forum) {
    $ret = $thread->update("hidden=1");
    if (!$ret) return $ret;
    $forum->update("threads=threads-1");
    $forum->update("posts=posts-$thread->replies-1");
    update_forum_timestamp($forum);
    return true;
}

function unhide_thread($thread, $forum) {
    $ret = $thread->update("hidden=0");
    if (!$ret) return $ret;
    $forum->update("threads=threads+1, posts=posts+$thread->replies+1");
    update_forum_timestamp($forum);
    return true;
}

function move_thread($thread, $old_forum, $new_forum) {
    $now = time();
    $old_forum->update("threads=threads-1, posts=posts-$thread->replies-1");
    $new_forum->update("threads=threads+1, posts=posts+$thread->replies+1, timestamp=$now");
    return $thread->update("forum=$new_forum->id");
}

// $show_hidden: 1 if it is a moderator reading
// Error page if this function returns NULL.
// $forumID - int
// $min - int
// $nRec - int
// $sort_style - string (checked by switch statement)
// $show_hidden - bool (not directly passed to SQL)
// $sticky - bool (not directly passed to SQL)
//
function get_forum_threads(
    $forumID, $start=-1, $nRec=-1, $sort_style=MODIFIED_NEW,
    $show_hidden = 0, $sticky = 1
) {
    //if (! (is_numeric($forumID) && is_numeric($min) && is_numeric($nRec))) {
    //    return NULL;  // Something is wrong here.
    //}
        
    $sql = 'forum = ' . $forumID ;
    if ($sticky){
        $stickysql = "sticky DESC, ";
    }
    if (!$show_hidden) {
        $sql .= ' AND hidden = 0';
    }
    switch($sort_style) {
    case MODIFIED_NEW:
        $sql .= ' ORDER BY '.$stickysql.'timestamp DESC';
        break;
    case MODIFIED_OLD:
        $sql .= ' ORDER BY '.$stickysql.'timestamp ASC';
        break;
    case VIEWS_MOST:
        $sql .= ' ORDER BY '.$stickysql.'views DESC';
        break;
    case REPLIES_MOST:
        $sql .= ' ORDER BY '.$stickysql.'replies DESC';
        break;
    case CREATE_TIME_NEW:
        $sql .= ' ORDER by '.$stickysql.'create_time desc';
        break;
    case CREATE_TIME_OLD:
        $sql .= ' ORDER by '.$stickysql.'create_time asc';
        break;
    case 'sufferers':
        $sql .= ' ORDER by '.$stickysql.'sufferers desc';
        break;
    case 'activity':
        $sql .= ' ORDER by '.$stickysql.'activity desc';
        break;
    case 'score':
        $sql .= ' ORDER by '.$stickysql.'score desc';
        break;
    default:
        $sql .= ' ORDER BY '.$stickysql.'timestamp DESC';
        break;
    }
    if ($start > -1) {
        $sql .= ' LIMIT '.$start;
        if ($nRec > -1) {
            $sql .= ', '.$nRec;
        }
    } else if ($nRec > -1) {
        $sql .= ' LIMIT '.$nRec;
    }
    return BoincThread::enum($sql);
}

// $show_hidden = true when it is a moderator reading
// error_page if this function returns NULL.
// $sort_style - string (checked by switch statement)
// $show_hidden - bool (not directly passed to SQL)
//
function get_thread_posts($threadid, $sort_style, $show_hidden) {
    $sql = "thread=$threadid";
    if (!$show_hidden) {
        $sql .= ' AND hidden = 0';
    }
    switch($sort_style) {
    case CREATE_TIME_NEW:
        $sql .= ' ORDER BY timestamp desc';
        break;
    case CREATE_TIME_OLD:
        $sql .= ' ORDER BY timestamp asc';
        break;
    case POST_SCORE:
        $sql .= ' ORDER BY score DESC';
        break;
    default:
        $sql .= ' ORDER BY timestamp asc';
        break;
    }
    return BoincPost::enum($sql);
}

// Show the links for possible moderation actions related to a single post 
//
function show_post_moderation_links(
    $config, $logged_in_user, $post, $forum, $tokens
){
    $moderators_allowed_to_ban = parse_bool($config, "moderators_allowed_to_ban"); 
    $moderators_vote_to_ban = parse_bool($config, "moderators_vote_to_ban");

    if ($post->hidden) {
        show_button("forum_moderate_post_action.php?action=unhide&amp;id=".$post->id."$tokens", tra("Unhide"), tra("Unhide this post"));
    } else {
        show_button("forum_moderate_post.php?action=hide&amp;id=".$post->id."$tokens", tra("Hide"), tra("Hide this post"));
    }

    show_button(
        "forum_moderate_post.php?action=move&amp;id=".$post->id."$tokens",
        tra("Move"), tra("Move post to a different thread")
    );

    if ($forum->parent_type == 0) {
        if ($logged_in_user->prefs->privilege(S_ADMIN) || ($logged_in_user->prefs->privilege(S_MODERATOR) && $moderators_allowed_to_ban)) {
            show_button("forum_moderate_post.php?action=banish_user&amp;id=".$post->id."&amp;userid=".$post->user."$tokens", tra("Banish author"));
        }
        if ($logged_in_user->prefs->privilege(S_MODERATOR) && $moderators_vote_to_ban) {
            require_once("../inc/forum_banishment_vote.inc");
            if (vote_is_in_progress($post->user)) {
                show_button(
                    "forum_banishment_vote.php?action=yes&amp;userid=".$post->user,
                    tra("Vote to banish author")
                );
                show_button(
                    "forum_banishment_vote.php?action=no&amp;userid=".$post->user,
                    tra("Vote not to banish author")
                );
            } else {
                show_button(
                    "forum_banishment_vote.php?action=start&amp;userid=".$post->user,
                    tra("Start vote to banish author")
                );
            }
        }
    }
}

function user_can_create_thread($user, $forum) {
    if (!$user) return false;
    if ($forum->is_dev_blog){
        if (
           (!$user->prefs->privilege(S_SCIENTIST)) &&
           (!$user->prefs->privilege(S_DEV)) &&
           (!$user->prefs->privilege(S_ADMIN))
        ) {
            return false;
        }
    }
    return true;
}

function check_post_access($user, $forum) {
    switch ($forum->parent_type) {
    case 0:
        if ($user->prefs->privilege(S_MODERATOR)) return;
        break;
    case 1:
        $team = BoincTeam::lookup_id($forum->category);
        if (is_team_admin($user, $team)) return;

        // non-team-members can't post
        //
        if ($user->teamid != $team->id) {
            error_page(tra("Only team members can post to the team message board"));
        }
        break;
    }

    // If user haven't got enough credit (according to forum regulations)
    // We do not tell the (ab)user how much this is -
    // no need to make it easy for them to break the system.
    //
    if ($user->total_credit<$forum->post_min_total_credit || $user->expavg_credit<$forum->post_min_expavg_credit) {
        error_page(tra("In order to create a new thread in %1 you must have a certain amount of credit. This is to prevent and protect against abuse of the system.", $forum->title));
    }

    // If the user is posting faster than forum regulations allow
    // Tell the user to wait a while before creating any more posts
    //
    if (time()-$user->prefs->last_post <$forum->post_min_interval) {
        error_page(tra("You cannot create any more threads right now. Please wait a while before trying again. This delay has been enforced to protect against abuse of the system."));
    }
}

function check_reply_access($user, $forum, $thread) {
    if ($thread->locked && !is_moderator($user, $forum)) {
        error_page(
            tra("This thread is locked. Only forum moderators and administrators are allowed to post there.")
        );
    }
    if ($thread->hidden) {
        error_page(
           tra("Can't post to a hidden thread.")
        );
    }

    check_post_access($user, $forum);
}

function is_moderator($user, $forum) {
    if (!$user) return false;
    $type = $forum?$forum->parent_type:0;
    switch ($type) {
    case 0:
        if ($user->prefs->privilege(S_MODERATOR)) return true;
        if ($user->prefs->privilege(S_ADMIN)) return true;
        if ($user->prefs->privilege(S_DEV)) return true;
        if ($user->prefs->privilege(S_SCIENTIST)) return true;
        break;
    case 1:
        if ($user->prefs->privilege(S_ADMIN)) return true;
        $team = BoincTeam::lookup_id($forum->category);
        return is_team_admin($user, $team);
        break;
    }
    return false;
}

function show_thread_and_context_header() {
    start_forum_table(array(
        tra("Thread"),
        tra("Posts"),
        tra("Author"),
        tra("Views"),
        "<nobr>".tra("Last post")."</nobr>"
    ));
}

// show a 1-line summary of thread and its forum.
// Used for search results and subscription list
//
function show_thread_and_context($thread, $user, $i) {
    $thread_forum = BoincForum::lookup_id($thread->forum);
    if (!$thread_forum) return;
    if (!is_forum_visible_to_user($thread_forum, $user)) return;
    $owner = BoincUser::lookup_id($thread->owner);
    $j = $i % 2;
    echo "<tr class=row$j><td>\n";
    switch($thread_forum->parent_type) {
    case 0:
        $category = BoincCategory::lookup_id($thread_forum->category);
        show_forum_title($category, $thread_forum, $thread, true);
        break;
    case 1:
        show_team_forum_title($thread_forum, $thread);
        break;
    }
    echo '
        </td><td class="numbers">'.($thread->replies+1).'</td>
        <td>'.user_links($owner).'</td>
        <td class="numbers">'.$thread->views.'</td>
        <td class="lastpost">'.time_diff_str($thread->timestamp, time()).'</td>
        </tr>
    ';
}

// see if thread is in subscription list
//
function is_subscribed($thread, $subs) {
    foreach ($subs as $sub) {
        if ($sub->threadid == $thread->id) return true;
    }
    return false;
}

function is_forum_visible_to_user($forum, $user) {
    if ($forum->parent_type == 1) {
        if (parse_config(get_config(), "<team_forums_members_only>")) {
            if (!$user) return false;
            if ($user->teamid != $forum->category) return false;
        }
    }
    return true;
}

function subscribed_post_email_line($notify) {
    $thread = BoincThread::lookup_id($notify->opaque);
    return "There are new posts in the thread '$thread->title'";
}

function subscribed_post_web_line($notify) {
    $thread = BoincThread::lookup_id($notify->opaque);
    return tra("New posts in the thread %1","<a href=forum_thread.php?id=$thread->id>$thread->title</a>");
}

function subscribe_rss($notify, &$title, &$msg, &$url) {
    $thread = BoincThread::lookup_id($notify->opaque);
    $title = tra("New posts in subscribed thread");
    $msg = tra("There are new posts in the thread '%1'",$thread->title);
    $url = URL_BASE."forum_thread.php?id=$thread->id";
}

function show_mark_as_read_button($user) {
    if ($user) {
        $return = urlencode(current_url());
        $tokens = url_tokens($user->authenticator);
        $url = "forum_index.php?read=1$tokens&amp;return=$return";
        show_button($url,
            tra("Mark all threads as read"),
            tra("Mark all threads in all message boards as read.")
        );
    }
}

function remove_subscriptions_forum($userid, $forumid) {
    $subs = BoincSubscription::enum("userid=$userid");
    foreach ($subs as $sub) {
        $thread = BoincThread::lookup_id($sub->threadid);
        if ($thread && $thread->forum == $forumid) {
            BoincSubscription::delete($userid, $thread->id);
        }
    }
    $notices = BoincNotify::enum("userid=$userid and type=".NOTIFY_SUBSCRIBED_POST);
    foreach ($notices as $n) {
        $thread = BoincThread::lookup_id($n->opaque);
        if ($thread && $thread->forum == $forumid) {
            $n->delete();
        }
    }
}

function remove_subscriptions_thread($userid, $threadid) {
    BoincSubscription::delete($userid, $threadid);
    BoincNotify::delete_aux("userid=$userid and type=".NOTIFY_SUBSCRIBED_POST." and opaque=$threadid");
}

?>
