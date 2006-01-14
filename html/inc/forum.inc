<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once('../inc/db.inc');
require_once('../inc/db_forum.inc');
require_once('../inc/time.inc');
require_once('../inc/forum_moderators.inc');
require_once('../inc/text_transform.inc');

define('AVATAR_WIDTH', 100);
define('AVATAR_HEIGHT',100);

$special_user_bitfield[0]="Forum moderator";
$special_user_bitfield[1]="Project administrator";
$special_user_bitfield[2]="Project developer";
$special_user_bitfield[3]="Project tester";
$special_user_bitfield[4]="Volunteer developer";
$special_user_bitfield[5]="Volunteer tester";
$special_user_bitfield[6]="Project scientist";

define('ST_NEW_TIME', 1209600); //3600*24*14 - 14 days
define('ST_NEW', 'New member');

define('MAXIMUM_EDIT_TIME',3600); //Maximally allow edits of forums posts up till one hour after posting.

define('FORUM_OPEN_LINK_IN_NEW_WINDOW',1);
define('MAX_FORUM_LOGGING_TIME', 604800); //3600*24*7 - 7 days
define('NO_CONTROLS', 0);
define('FORUM_CONTROLS', 1);
define('HELPDESK_CONTROLS', 2);
define("EXCERPT_LENGTH", "120");

define('NEW_IMAGE', 'img/unread_post.png');
define('NEW_IMAGE_STICKY', 'img/unread_sticky.png');
define('STICKY_IMAGE', 'img/sticky_post.png');
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

$forum_sort_styles['modified-new'] = "Most recent post first";
$forum_sort_styles['modified-old'] = "Least recent post first";
//$forum_sort_styles['activity-most'] = "Most recent activity first";
$forum_sort_styles['views-most'] = "Most views first";
$forum_sort_styles['replies-most'] = "Most posts first";

$thread_sort_styles['timestamp'] = "Newest first";
$thread_sort_styles['timestamp_asc'] = "Oldest first";
$thread_sort_styles['score'] = "Highest rated first";

$faq_sort_styles['create_time'] = "Most recent question first";
$faq_sort_styles['timestamp'] = "Most recent answer first";
$faq_sort_styles['activity'] = "Most frequently asked first";

$answer_sort_styles['score'] = "Highest score first";
$answer_sort_styles['timestamp'] = "Most recent first";
$answer_sort_styles['timestamp_asc'] = "Oldest first";

$thread_filter_styles['2'] = "\"Very helpful\"";
$thread_filter_styles['1'] = "At least \"helpful\"";
$thread_filter_styles['0'] = "At least \"neutral\"";
$thread_filter_styles['-1'] = "At least \"unhelpful\"";
$thread_filter_styles['-2'] = "All posts";

$post_ratings['2'] = "Very helpful (+2)";
$post_ratings['1'] = "Helpful (+1)";
$post_ratings['0'] = "Neutral";
$post_ratings['-1'] = "Not helpful (-1)";
$post_ratings['-2'] = "Off topic (-2)";

// process a user-supplied title to remove HTML stuff
//
function cleanup_title($title) {
    $x = trim(htmlspecialchars(strip_tags($title)));
    $x = stripslashes($x);      // clean up funky old titles in DB
    if (strlen($x)==0) return "(no title)";
    else return $x;
}

/* Check if user has special user bit enabled */
function isSpecialUser($user, $specialbit){
    return (substr($user->special_user, $specialbit,1)==1);
}

function getHasRated($user, $postid){
    return (strstr($user->rated_posts,"|".$postid));
}



function getSortStyle($user,$place){
    if ($user->id!=""){
        list($forum,$thread,$faq,$answer)=explode("|",$user->sorting);
    } else {
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
    }
    return $$place;
}


// display functions

function show_posts($thread, $sort_style, $filter, $show_controls=true, $do_coloring=true, $is_helpdesk=false) {
    global $logged_in_user;
    $n = 1;

    if ($show_controls && !$is_helpdesk) {
        $controls = FORUM_CONTROLS;
    } else if ($show_controls && $is_helpdesk) {
        $controls = HELPDESK_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }

    // If logged in user is moderator,
    // let him see all posts - including hidden ones
    //
    if (isSpecialUser($logged_in_user,0)){
        $show_hidden_posts = true;
    } else {
        $show_hidden_posts = false;
    }

    $posts = getPosts($thread->id, -1, -1, $sort_style, $show_hidden_posts);
    $postcount = mysql_num_rows($posts);
    $logged_in_user = getThreadLastVisited($logged_in_user,$thread);
    setThreadLastVisited($logged_in_user,$thread);

    $firstPost = getFirstPost($thread->id);

    $postnumber=0; $previous_post=0;
    if ($is_helpdesk) {
        if ($firstPost) {
            show_post($firstPost, $thread, $logged_in_user, $n, $controls, true,$filter);
	    $postnumber=1; $previous_post=1;
            if ($firstPost->timestamp>$logged_in_user->thread_last_visited){
                $first_unread_post=$firstPost;
            }
        }
    }
    
    //$logged_in_user->minimum_wrap_postcount=13;
    //$logged_in_user->display_wrap_postcount=12;
    $no_wraparound = get_str("nowrap",true);
    while ($post = mysql_fetch_object($posts)) {
        if (!$is_helpdesk || ($is_helpdesk && $post->id != $firstPost->id)) {
	    $postnumber++;
	    
	    if (
		(!$logged_in_user->minimum_wrap_postcount) //If the user hasn't enabled the feature to display only a certain amount of posts, simply display all of them.
		|| ($postcount <= $logged_in_user->minimum_wrap_postcount) //If it is enabled and we havent yet hit the limit, simply display all
		|| ($no_wraparound)  // If the user has asked to display all the posts just display them
		||
		(
		    ($postcount > $logged_in_user->minimum_wrap_postcount) //If it is enabled and we have hit the limit AND we have the minimum wraparound number of posts or more
		    &&
		    (
			($postnumber==1 || $postnumber==$postcount) // Let's display the post only if it is the first post 
			|| 
			(	
			    (($postnumber > $postcount-$logged_in_user->display_wrap_postcount) //or it is one of the last wrap_display_postcount posts.
			    && ($sort_style=="timestamp_asc"))
			    ||
			    (($postnumber <= $logged_in_user->display_wrap_postcount) //or it is one of the last wrap_display_postcount posts.
			    && ($sort_style=="timestamp"))
			    || (substr($sort_style,0,4)!="time")
			)
			|| ($post->timestamp > $logged_in_user->thread_last_visited) //or if this post is unread
		    )
		)
	    ){
		if ($postnumber!=$previous_post+1){
		    //A number of posts were hidden, display a way to unhide them:
		    echo "<tr class=\"postseperator\"><td></td><td colspan=2>
		    Only the first post and the last ".($logged_in_user->display_wrap_postcount)." posts 
		    (of the ".($postcount)." posts in this thread) 
		    are displayed. <br />
		    <a href=\"?id=".$thread->id."&amp;nowrap=true\">Click here to also display the remaining posts</a>.</td></tr>";
		}
		$previous_post=$postnumber;
		show_post($post, $thread, $logged_in_user, $n, $controls, false, $filter);
        	if ($do_coloring) $n = ($n+1)%2;
        	if (($post->timestamp>$logged_in_user->thread_last_visited) && (($post->timestamp<$first_unread_post->timestamp) || $first_unread_post->timestamp==0)){
		    $first_unread_post=$post;
        	}
	    }
        }
    }

    if ($logged_in_user->jump_to_unread){
        if ($first_unread_post->id!=""){
            echo "<script>function jumpToUnread(){location.href='#".$first_unread_post->id."';}</script>";
        } else {
            echo "<script>function jumpToUnread(){};</script>";
        }
    }
}

function show_post($post, $thread, $logged_in_user, $n, $controls=FORUM_CONTROLS, $separate=false, $filter=true) {
    $user = lookup_user_id($post->user);
    $user = getForumPreferences($user);
    $user->has_avatar = ($user->avatar != "");

    //If the user that made this post is on the list of people to ignore, change thresholds to be more strict

    if (in_array($user->id,explode("|",$logged_in_user->ignorelist))){
        $user_is_on_ignorelist=true;
        $rated_below_threshold = ($logged_in_user->high_rating_threshold>($post->score*$post->votes));
        $rated_above_threshold = ($logged_in_user->high_rating_threshold+abs($logged_in_user->low_rating_threshold)<($post->score*$post->votes));
    } else {                //Use normal threshold values
        $rated_below_threshold = ($logged_in_user->low_rating_threshold>($post->score*$post->votes));
        $rated_above_threshold = ($logged_in_user->high_rating_threshold<($post->score*$post->votes));
    }


    $can_edit = ($logged_in_user && $user->id == $logged_in_user->id); //Only the creator can edit the post
    //$can_edit = ((isSpecialUser($logged_in_user,0)) || ($logged_in_user && $user->id == $logged_in_user->id));// Everybody assumes we can edit posts, why not make it so?
    echo "
        <tr class=\"row$n\" valign=\"top\">
        <td rowspan=\"3\">\n\t\t<div class=\"authorcol\">
        <a name=\"$post->id\"></a>
    ";

    echo user_links($user, URL_BASE);
    echo "<br>";
    global $special_user_bitfield; 
    $fstatus="";
    if ($user->special_user) {
        $keys = array_keys($special_user_bitfield);
        for ($i=0; $i<sizeof($special_user_bitfield);$i++) {
            if (isSpecialUser($user,$keys[$i])) {
                $fstatus.=$special_user_bitfield[$keys[$i]]."<br>";
            }
        }
    } else {
        if ($user->create_time>time()-ST_NEW_TIME) {
            $fstatus=ST_NEW."<br>";
        }
    }
    if ($fstatus) echo "<font size=\"-2\">$fstatus</font>";

    echo "<font size=-2>";
    if (!$filter || !$rated_below_threshold){
        if ($user->has_avatar and $logged_in_user->hide_avatars!=1) {
            echo "<img width=\"".AVATAR_WIDTH."\" height=\"".AVATAR_HEIGHT."\" src=\"".$user->avatar."\" alt=\"Avatar\"><br>";
        }
        echo "Joined: ", gmdate('M j, Y', $user->create_time), "<br>Posts: $user->posts<br>";
    }

    // circumvent various forms of identity spoofing
    // by displaying the  user id of the poster.
    // its cheap, easy, and doesn't require any additional database calls.
    echo "ID: $user->id<br>";
    // printf("Credit: %.0f<br>", $user->total_credit);
    echo "Credit: ".number_format($user->total_credit)."<br>";

    // printf("RAC: %.1f<br>", $user->expavg_credit);
    echo "RAC: ".number_format($user->expavg_credit)."<br>";

    echo "</font></div></td>\n\t\t
    ";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "<form action=\"forum_rate.php?post=", $post->id, "\" method=\"post\">";
    }
    if ($controls == FORUM_CONTROLS){
	echo "<td align=left colspan=2 class=\"postheader\" valign=\"top\"><font size=-2>";
    } else {
	echo "<td align=left class=\"postheader\" valign=\"top\"><font size=-2>";
    }

    if ($post->timestamp>$logged_in_user->thread_last_visited){
        echo "<img src=\"".NEW_IMAGE."\" alt=\"Unread post\" height=\"".NEW_IMAGE_HEIGHT."\">";
    }
    if ($rated_above_threshold){
        echo "<img src=\"".EMPHASIZE_IMAGE."\" alt=\"!\" height=\"".EMPHASIZE_IMAGE_HEIGHT."\">";
    }

    echo " <a href=\"#$post->id\">Message $post->id</a> - ";
    if ($post->hidden) echo "<font color=red>[deleted] </font>";
    echo "
        Posted ", pretty_time_str($post->timestamp);
    ;

    if ($post->parent_post) echo " - in response to <a href=\"#$post->parent_post\">Message ID $post->parent_post</a>.";
    if ($can_edit && $controls != NO_CONTROLS) echo "&nbsp;<a href=\"forum_edit.php?id=$post->id\">[Edit this post]</a>";
    if (isSpecialUser($logged_in_user,0)) show_post_moderation_links($post); //If user is moderator, show links
    if ($post->modified) echo "<br>Last modified: ", pretty_time_Str($post->modified);
    if ($rated_below_threshold && $filter){
        if ($user_is_on_ignorelist) $andtext=" and the user is on your ignore list";
        echo "<br>This post has been filtered (rating: ".($post->score * $post->votes).")$andtext, press <a href=\"?id=".$thread->id."&amp;filter=false#".$post->id."\">here</a> to view this thread without filtering";
    }
    echo "\n</font></td>\n";

    if ($controls == FORUM_CONTROLS) {
        //no special controls in forum
	//echo "<td class=\"postheader\"></td>";
    } else if ($controls == HELPDESK_CONTROLS && $separate) {
        echo "
            <td align=right class=\"postheader\">
            <input type=submit name=submit value=\"", SUFFERER, "\">
            </td>
        ";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo "
            <td align=right class=\"postheader\">
            <input type=submit name=submit value=\"", SOLUTION, "\">
            <input type=submit name=submit value=\"", OFF_TOPIC, "\">
            </td>
        ";
    }

    echo "</tr>\n";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "</form>";
    }
    echo "<tr class=\"row$n\"><td class=\"postbody\" colspan=\"2\">";
    //If either filtering is turned off of this post is not below the threshold
    if (!$filter || !$rated_below_threshold){
        $posttext = $post->content;

        // If the creator of this post has a signature and
        // wants it to be shown for this post AND the logged in
        // user has signatures enabled: show it
        //
        if ($post->signature && !$logged_in_user->hide_signatures){
            $posttext.="\n____________\n".$user->signature;
        }

        $options = get_transform_settings_from_user($logged_in_user);
        $posttext = output_transform($posttext,$options);
        
        echo "<p>", $posttext, "</p>";
        echo "</td></tr><tr><td class=\"postfooter\"><font size=-2><i>ID: ", $post->id;
        if ($controls == HELPDESK_CONTROLS && $separate) {
            echo "</i></font></td>";
        } else if ($controls == HELPDESK_CONTROLS && !$separate) {
            echo " / Score: ", round(($post->score * $post->votes),0), "</i></font></td>";
        } else {
            $rating = floor($post->score*$post->votes + 0.01);
            echo " | Rating: ", $rating, "</i> | rate: <a href=\"forum_rate.php?post=".$post->id."&amp;choice=p\"><img src=\"".RATE_POSITIVE_IMAGE."\" alt=\"+\" height=\"".RATE_POSITIVE_IMAGE_HEIGHT."\" border=0></a> / <a href=\"forum_rate.php?post=".$post->id."&amp;choice=n\"><img src=\"".RATE_NEGATIVE_IMAGE."\" alt=\"-\" height=\"".RATE_NEGATIVE_IMAGE_HEIGHT."\" border=0></a> - <a href=\"forum_report_post.php?post=".$post->id."\"><img src=\"".REPORT_POST_IMAGE."\" alt=\"Report this post as offensive\" height=\"".REPORT_POST_IMAGE_HEIGHT."\" border=0></a></font></td>";
        }

        if ($controls == FORUM_CONTROLS) {
            echo "<td align=right class=\"postfooter\">[<a href=\"forum_reply.php?thread=" . $thread->id . "&amp;post=" . $post->id . "#input\">Reply to this post</a>]</td>";
        } else if ($controls == HELPDESK_CONTROLS && !$separate) {
            echo "<td align=right class=\"postfooter\">[<a href=\"forum_reply.php?thread=" . $thread->id . "&amp;post=" . $post->id . "&helpdesk=1#input\">Reply to this answer</a>]</td>";
        } else {
	    echo "<td class=\"postfooter\"></td>";
	}
        echo "</tr>";
    }
    echo "</td></tr>";
    echo "<tr class=\"postseperator\"><td colspan=3></td></tr>";
    if ($separate) {
        $headings = array(array("Author","authorcol"), "Message","");
	echo "<tr class=\"helpdeskseperator\"><td></td></tr>";
        start_forum_table($headings, "id=\"thread\" width=100%");
    }
}

// utility functions

function start_forum_table($headings, $extra="width=100%") {
    start_table($extra." cellspacing=0");
    echo "<tr>";

    for ($i=0; $i<count($headings); $i++) {
        if (is_array($headings[$i])){
            $title = $headings[$i][0];
            $class = $headings[$i][1];
        } else {
            $title = $headings[$i];
            $class = "heading";
        }
        echo "<th class=$class>$title</th>";
    }
    echo "</tr>\n";
}

function end_forum_table() {
    echo "</table>\n";
}

// generate a "select" element from an array of values
//
function select_from_array($name, $array, $selection) {
    $out= "<select name=\"$name\">";

    foreach ($array as $key => $value) {
        $out.= "<option ";
        if ($key == $selection) {
            $out.= "selected ";
        }
        $out.= "value=\"". $key. "\">". $value. "</option>";
    }
    $out.= "</select>";
    return $out;
}

function show_select_from_array($name, $array, $selection) {
    echo select_from_array($name,$array,$selection);
}

function show_forum_title($forum=NULL, $thread=NULL, $helpdesk=false) {
    echo "<p>\n";
    if (!$forum && !$thread) {
        echo "<p class=\"title\">";
        if ($helpdesk) {
            echo " Questions and problems</p>";
        } else {
            echo " Message boards</p>";
        }
    } else if ($forum && !$thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=\"forum_help_desk.php\">", " Questions and problems</a> : ";
        } else {
            echo "<a href=\"forum_index.php\">", " Message boards</a> : ";
        }
        echo $forum->title;
        echo "</span><br>";
    } else if ($forum && $thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=\"forum_help_desk.php\">", " Questions and problems</a> : ";
        } else {
            echo "<a href=\"forum_index.php\">", " Message boards</a> : ";
        }
        echo "<a href=\"forum_forum.php?id=$forum->id\">", $forum->title, "</a> : ";
        echo cleanup_title($thread->title);
        echo "</span><br>";
    } else {
        echo "Invalid input to show_forum_title<br>";
    }
    echo "</p>\n";
}

// show a thread with its context (e.g. for search results)
//
function show_thread($thread, $n) {
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $first_post = getFirstPost($thread->id);
    $title = cleanup_title($thread->title);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"forum_help_desk.php":"forum_index.php";
    $excerpt = sub_sentence(stripslashes($first_post->content), ' ', EXCERPT_LENGTH, true);
    $posted = time_diff_str($thread->create_time, time());
    $last = time_diff_str($thread->timestamp, time());
    $m = $n%2;
    echo "
        <tr class=\"row$m\">
        <td><font size=\"-2\">
            $n) Posted $posted
            <br>
            Last response $last
        </td>
        <td valign=top>
            <a href=\"$top_url\">$where</a> : $category->name :
            <a href=\"forum_forum.php?id=$forum->id\">$forum->title</a> :
            <a href=\"forum_thread.php?id=$thread->id\">$title</a>
            <br>
            <font size=\"-2\">$excerpt</font>
        </td>
        </tr>
    ";
}

// show a post with its context (e.g. for search results)
//
function show_post2($post, $n) {
    $thread = getThread($post->thread);
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"forum_help_desk.php":"forum_index.php";
    $content = output_transform($post->content);
    $when = time_diff_str($post->timestamp, time());
    $user = lookup_user_id($post->user);
    $title = cleanup_title($thread->title);
    $m = $n%2;
    if($post->hidden) {
        //Todo: factor this array out, it is also used elsewhere
        $deleted_text = array( "Obscene", "Flame/Hate", "Commercial spam" );
        $deleted = "<br><font color=red>[Deleted " .
                   "by a moderator as " . $deleted_text[$post->hidden-1] .
                   "] </font>";
    } else {
        $deleted = "";
    }
    echo "
        <tr class=row$m>
        <td>
            $n) <a href=\"$top_url\">$where</a> : $category->name :
            <a href=\"forum_forum.php?id=$forum->id\">$forum->title</a> :
            <a href=\"forum_thread.php?id=$thread->id\">$title</a>
            <br>
            Posted $when by $user->name $deleted
            <hr>
            $content
        </td>
        </tr>
    ";
}

function show_forum_summary($forum) {
    $x = time_diff_str($forum->timestamp, time());
    echo "
        <tr class=row1>
        <td>
            <b>
            <a href=forum_forum.php?id=$forum->id>", $forum->title,
            "</a></b>
            <br><font size=-2>", $forum->description, "
        </td>
        <td>", $forum->threads, "</td>
        <td>", $forum->posts, "</td>
        <td>", $x, "</td>
    </tr>
    ";
}

function post_warning() {
    return "<br><br><font size=-2>
        Don't use obscene language or images,
        and don't threaten other participants;
        otherwise we may delete your messages.</font>
    ";
}

?>
