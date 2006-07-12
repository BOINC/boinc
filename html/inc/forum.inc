<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once('../inc/db.inc');
require_once('../inc/db_forum.inc');
require_once('../inc/time.inc');
require_once('../inc/forum_moderators.inc');
require_once('../inc/text_transform.inc');
require_once('../inc/util.inc');

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
define('MAX_FORUM_LOGGING_TIME', 2419200); //3600*24*28 - 28 days
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

$config = get_config();
$no_forum_rating = parse_element($config,"<no_forum_rating/>"); 

/**
 *  Process a user-supplied title to remove HTML stuff
 **/
function cleanup_title($title) {
    $x = trim(htmlspecialchars(strip_tags($title)));
    $x = stripslashes($x);      // clean up funky old titles in DB
    if (strlen($x)==0) return "(no title)";
    else return $x;
}

/** 
 * Check if user has special user bit enabled
 **/
function isSpecialUser($user, $specialbit){
    return (substr($user->special_user, $specialbit,1)==1);
}

/**
 * Check to see if a specific user has rated a specific post before
 **/
function getHasRated($user, $postid){
    return (strstr($user->rated_posts,"|".$postid));
}

/**
 * Get the user's preferred sorting style.
 **/
function getSortStyle($user,$place){
    if ($user->id!=""){
        list($forum,$thread,$faq,$answer)=explode("|",$user->sorting);
    } else {
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
    }
    return $$place;    // Huh?
}


/*** display functions ***/

/**
 * Show the posts in a thread for a user.
 * It's a big one.
 **/
function show_posts($thread, $sort_style, $filter, $logged_in_user, $show_controls=true) {
    $n = 1;

    if ($show_controls) {
        $controls = FORUM_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }

    // If logged in user is moderator,
    // let him see all posts - including hidden ones
    if ($logged_in_user && $logged_in_user->isSpecialUser(S_MODERATOR)){
        $show_hidden_posts = true;
    } else {
        $show_hidden_posts = false;
    }
    $posts = $thread->getPosts($sort_style, $show_hidden_posts);
    $postcount = (sizeof($posts)-1);
    if ($logged_in_user) $last_visit = $thread->getLastReadTimestamp($logged_in_user);

    $postnumber=0; $previous_post=0;
    $no_wraparound = get_str("nowrap",true);
    foreach ($posts as $key => $post){
            $postnumber++;
            
            // There *HAS* to be a better way to do this.  WOW.
            if ((!$logged_in_user) // If the user isn't logged in
                || (!$logged_in_user->getMinimumWrapPostcount()) //or If the user hasn't enabled the feature to display only a certain amount of posts, simply display all of them.
                || ($postcount <= $logged_in_user->getMinimumWrapPostcount()) //If it is enabled and we havent yet hit the limit, simply display all
                || ($no_wraparound)  // If the user has asked to display all the posts just display them
                ||
                (
                    ($postcount > $logged_in_user->getMinimumWrapPostcount()) //If it is enabled and we have hit the limit AND we have the minimum wraparound number of posts or more
                    &&
                    (
                        ($postnumber==1 || $postnumber==($postcount+1)) // Let's display the post only if it is the first post 
                        || 
                        (        
                            (($postnumber > $postcount-$logged_in_user->getDisplayWrapPostcount()) //or it is one of the last wrap_display_postcount posts.
                            && ($sort_style==CREATE_TIME_OLD))
                            ||
                            (($postnumber <= $logged_in_user->getDisplayWrapPostcount()) //or it is one of the last wrap_display_postcount posts.
                            && ($sort_style==CREATE_TIME_NEW))
                            || ($sort_style!=CREATE_TIME_NEW && $sort_style!=CREATE_TIME_OLD)
                        )  
                        || ($post->getTimestamp() > $last_visit) //or if this post is unread
                    )
                )
            ) {
                if ($postnumber!=$previous_post+1){
                    //A number of posts were hidden, display a way to unhide them:
                    echo "<tr class=\"postseperator\"><td></td><td colspan=\"2\">
                    Only the first post and the last ".($logged_in_user->getDisplayWrapPostcount())." posts 
                    (of the ".($postcount)." posts in this thread) are displayed. <br />
                    <a href=\"?id=".$thread->getID()."&amp;nowrap=true\">Click here to also display the remaining posts</a>.</td></tr>";
                }
                $previous_post=$postnumber;
                show_post($post, $thread, $logged_in_user, $n, $controls, $filter);
                // To allow users to start reading a thread even though our DB layer is super slow
                $n = ($n+1)%2;
                
                if (($post->getTimestamp()>$last_visit) && 
                    ((!$first_unread_post) || ($post->getTimestamp()<$first_unread_post->getTimestamp()))
                   ){
                    $first_unread_post=$post;
                }
            }
    }

    if ($logged_in_user && $logged_in_user->hasJumpToUnread()){
        if ($first_unread_post){
            echo "<script>function jumpToUnread(){location.href='#".$first_unread_post->getID()."';}</script>";
        } else {
            echo "<script>function jumpToUnread(){};</script>";
        }
    }

    if ($logged_in_user) $thread->setLastReadTimestamp($logged_in_user);
}

/**
 * Display an individual post, as though in a thread.
 **/
function show_post($post, $thread, $logged_in_user, $n, $controls=FORUM_CONTROLS, $filter=true) {
    $user = $post->getOwner();
    global $no_forum_rating;

    //If the user that made this post is on the list of people to ignore, change thresholds to be much more strict
    if ($logged_in_user){
        if (in_array($user->getID(),$logged_in_user->getIgnoreList())){
            $user_is_on_ignorelist=true;
            $rated_below_threshold = ($logged_in_user->getHighRatingThreshold()>$post->getRating());
            $rated_above_threshold = ($logged_in_user->getHighRatingThreshold()+abs($logged_in_user->getLowRatingThreshold())<($post->getRating()));
        } else {                //Use normal threshold values
            $rated_below_threshold = ($logged_in_user->getLowRatingThreshold()>($post->getRating()));
            $rated_above_threshold = ($logged_in_user->getHighRatingThreshold()<($post->getRating()));
        }
    }
    // Only the creator can edit the post and only in a specified amount of time
    $can_edit = ($logged_in_user && $user->getID() == $logged_in_user->getID() && (time()<($post->getTimestamp()+MAXIMUM_EDIT_TIME-60))); 
    echo "
        <tr class=\"row$n\" valign=\"top\">
        <td rowspan=\"3\"><div class=\"authorcol\">
        <a name=\"".$post->getID()."\"></a>
    ";

    // Print the user links
    echo re_user_links($user, URL_BASE);
    echo "<br>";

    // Print the special user lines, if any
    global $special_user_bitfield; 
    $fstatus="";
    $keys = array_keys($special_user_bitfield);
    for ($i=0; $i<sizeof($special_user_bitfield);$i++) {
        if ($user->isSpecialUser($keys[$i])) {
            $fstatus.=$special_user_bitfield[$keys[$i]]."<br>";
        }
    }
    if ($user->create_time>time()-ST_NEW_TIME) $fstatus.=ST_NEW."<br>";
    if ($fstatus) echo "<font size=\"-2\">$fstatus</font>";

    echo "<span class=\"authorinfo\">";
    if (!$filter || !$rated_below_threshold){
        if ($user->hasAvatar() && (!$logged_in_user || ($logged_in_user->hasHideAvatars()==false))) {
            echo "<img width=\"".AVATAR_WIDTH."\" height=\"".AVATAR_HEIGHT."\" src=\"".$user->getAvatar()."\" alt=\"Avatar\"><br>";
        }
        echo "Joined: ", gmdate('M j, Y', $user->getCreateTime()), "<br>Posts: ".$user->getPostCount()."<br>";
    }
    // circumvent various forms of identity spoofing
    // by displaying the  user id of the poster.
    // its cheap, easy, and doesn't require any additional database calls.
    echo "ID: ".$user->getID()."<br>";
    echo "Credit: ".number_format($user->getTotalCredit())."<br>";
    echo "RAC: ".number_format($user->getExpavgCredit())."<br>";
    echo "</span></div></td>";

    if ($controls == FORUM_CONTROLS) {
        echo "<form action=\"forum_rate.php?post=", $post->getID(), "\" method=\"post\">";
        echo "<td colspan=\"2\" class=\"postheader\">";
    } else {
        echo "<td class=\"postheader\">";
    }

    if ($logged_in_user && $post->getTimestamp()>$thread->getLastReadTimestamp($logged_in_user)){
        echo "<img src=\"".NEW_IMAGE."\" alt=\"Unread post\" height=\"".NEW_IMAGE_HEIGHT."\">";
    }
    if ($rated_above_threshold){
        echo "<img src=\"".EMPHASIZE_IMAGE."\" alt=\"!\" height=\"".EMPHASIZE_IMAGE_HEIGHT."\">";
    }

    echo " <a href=\"forum_thread.php?id=".$thread->getID()."&nowrap=true#$post->id\">Message ".$post->getID()."</a> - ";
    if ($post->isHidden()) echo "<font color=red>[deleted] </font>";
    echo "
        Posted ", pretty_time_str($post->getTimestamp());
    ;

    if ($post->getParentPostID()) echo " - in response to <a href=\"forum_thread.php?id=".$thread->getID()."&nowrap=true#".$post->getParentPostID()."\">Message ID ".$post->getParentPostID()."</a>.";
    if ($can_edit && $controls != NO_CONTROLS) echo "&nbsp;<a href=\"forum_edit.php?id=".$post->getID()."\">[Edit this post]</a>";
    if ($logged_in_user && $logged_in_user->isSpecialUser(S_MODERATOR)) echo post_moderation_links($post); //If user is moderator, show links
    if ($post->getModificationTimestamp()) echo "<br>Last modified: ", pretty_time_Str($post->getModificationTimestamp());
    if ($rated_below_threshold && $filter){
        if ($user_is_on_ignorelist) $andtext=" and the user is on your ignore list";
        echo "<br>This post has been filtered (rating: ".($post->getRating()).")$andtext, press <a href=\"?id=".$thread->getID()."&amp;filter=false#".$post->getID()."\">here</a> to view this thread without filtering";
    }
    echo "</td></tr></form>";
    echo "<tr class=\"row$n\"><td class=\"postbody\" colspan=\"2\">";

    //If either filtering is turned off or this post is not below the threshold
    if (!$filter || !$rated_below_threshold){
        $posttext = $post->getContent();

        // If the creator of this post has a signature and
        // wants it to be shown for this post AND the logged in
        // user has signatures enabled: show it
        //
        if ($post->hasSignature() && (!$logged_in_user || !$logged_in_user->hasHideSignatures())){
            $posttext.="\n____________\n".$user->getSignature();
        }

        if ($logged_in_user){
            $options = $logged_in_user->getTextTransformSettings();
        } else {
            $options = new output_options;
        }
        $posttext = output_transform($posttext,$options);
        
        echo "<p>", $posttext, "</p>";
        echo "</td></tr><tr><td class=\"postfooter\">ID: <i>", $post->id;
        if ($no_forum_rating != NULL) { 
            echo " | <a href=\"forum_report_post.php?post=".$post->getID()."\"><img src=\"".REPORT_POST_IMAGE."\" alt=\"Report this post as offensive\" height=\"".REPORT_POST_IMAGE_HEIGHT."\" border=0></a></td>";
        } else {
        $rating = $post->getRating();
        echo " | Rating: ", $rating, "</i> | rate: <a href=\"forum_rate.php?post=".$post->getID()."&amp;choice=p\"><img src=\"".RATE_POSITIVE_IMAGE."\" alt=\"+\" height=\"".RATE_POSITIVE_IMAGE_HEIGHT."\" border=0></a> / <a href=\"forum_rate.php?post=".$post->getID()."&amp;choice=n\"><img src=\"".RATE_NEGATIVE_IMAGE."\" alt=\"-\" height=\"".RATE_NEGATIVE_IMAGE_HEIGHT."\" border=0></a> - <a href=\"forum_report_post.php?post=".$post->getID()."\"><img src=\"".REPORT_POST_IMAGE."\" alt=\"Report this post as offensive\" height=\"".REPORT_POST_IMAGE_HEIGHT."\" border=0></a></td>";
        }
        if ($controls == FORUM_CONTROLS) {
            echo "<td align=right class=\"postfooter\">[<a href=\"forum_reply.php?thread=" . $thread->getID() . "&amp;post=" . $post->getID() . "#input\">Reply to this post</a>]</td>";
        } else {
            echo "<td class=\"postfooter\"></td>";
        }
        echo "</tr>";
    }
    echo "</td></tr>";
    echo "<tr class=\"postseperator\"><td colspan=3></td></tr>";
}

/*** utility functions ***/

/**
 * Start the forum table, output the proper headings and such.
 **/
function start_forum_table($headings, $extra="width=100%") {
    start_table($extra." cellspacing=0 cellpadding=2");
    echo "<tr>";

    for ($i=0; $i<count($headings); $i++) {
        if (is_array($headings[$i])){
            $title = $headings[$i][0];
            $class = $headings[$i][1]?$headings[$i][1]:"heading";
            if ($headings[$i][2]) $span = " colspan=\"".$headings[$i][2]."\" ";
        } else {
            $title = $headings[$i];
            $class = "heading";
            $span="";
        }
        echo "<th class=$class$span>$title</th>";
    }
    echo "</tr>\n";
}

/**
 * End the forum table, currently just close the open table tag.
 **/
function end_forum_table() {
    echo "</table>\n";
}

/**
 * generate a "select" HTML element from an array of values
 **/
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

/**
 * Call and output the above.  Why is this a function, exactly?
 **/
function show_select_from_array($name, $array, $selection) {
    echo select_from_array($name,$array,$selection);
}

/**
 * Output the forum/thread title.
 **/
function show_forum_title($forum=NULL, $thread=NULL) {
    echo "<p>\n";
    $category = getCategory($forum->category);
    $where = $category->is_helpdesk?tr(LINKS_QA):tr(FORUM_TITLE_SHORT);
    $top_url = $category->is_helpdesk?"forum_help_desk.php":"forum_index.php";
    if (!$forum && !$thread) {
        echo "<p class=\"title\">";
        echo $where;

    } else if ($forum && !$thread) {
        echo "<span class=title>";
        echo "<a href=\"$top_url\">$where</a> : ";
        echo $forum->getTitle();
        echo "</span><br>";
    } else if ($forum && $thread) {
        echo "<span class=title>";
        echo "<a href=\"$top_url\">$where</a> : ";

        echo "<a href=\"forum_forum.php?id=".$forum->getID()."\">", $forum->getTitle(), "</a> : ";
        echo cleanup_title($thread->getTitle());
        echo "</span><br>";
    } else {
        echo "Invalid input to show_forum_title<br>"; // TODO: handle this condition gracefully 
    }
    echo "</p>\n";
}

/**
 * show a thread with its context (e.g. for search results)
 **/
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

/**
 * Show a post with its context (e.g. for search results)
 **/
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
        $deleted_text = array( "Obscene", "Flame/Hate", "Commercial spam", "Flamebait", "Doublepost", "Other");
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


function post_rules() {
    return "
        <ul>
        <li> No commercial advertisements.
        <li> No messages whose only intention is to annoy or antagonize other people.
        <li> No messages whose only intention is to hijack a thread.
        <li> No messages that are deliberately hostile or insulting.
        <li> No abusive comments involving race, religion,
        nationality, gender, class or sexuality.
        <li> No obscene language.
        </ul>
    ";
}

function post_warning() {
    return "<br><br>
        <table><tr><td>
        <font size=-2>
        Rules:
    ".post_rules()."
        <a href=moderation.php>More info</a>
        </font>
        </td></tr></table>
    ";
}

?>
