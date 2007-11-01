<?php
/**
 * This page displays the threads in a forum.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');

db_init();

$id = get_int("id");
$sort_style = get_int("sort", true);
$start = get_int("start", true);
if (!$start) $start = 0;

$forum = new Forum($id);
$user = re_get_logged_in_user(false);

if (!$sort_style) {
    // get the sort style either from the logged in user or a cookie
    if ($user){
        $sort_style = $user->getForumSortStyle();
    } else {
        if (isset($_COOKIE['sorting'])) {
            list($sort_style,$thread_style)=explode("|",$_COOKIE['sorting']);
        }
    }
} else {
    // set the sort style
    if ($user){
        $user->setForumSortStyle($sort_style);
    } else {
        list($old_style,$thread_style)=explode("|",$_COOKIE['sorting']);
        setcookie('sorting', implode("|",array($sort_style,$thread_style)), time()+3600*24*365);
    }
}

$Category = $forum->getCategory(); 
if ($Category->getType()!=0){
    page_head(tr(LINKS_QA).' : '.$forum->getTitle());
} else {
    page_head(tr(FORUM_TITLE_SHORT).' : '.$forum->getTitle());
}

// Allow users with a linktab-browser to get some usefull links
echo '<link href="forum_index.php" rel="up" title="Forum Index">';

show_forum_title($forum, NULL);

echo '
    <table width="100%" cellspacing="0" cellpadding="0">
    <tr valign="bottom">
    <td colspan=2>
';

show_button("forum_post.php?id=$id", "New thread", "Add a new thread to this forum");

if ($user) {
    $return = urlencode(current_url());
    $tokens = url_tokens($user->dbObj->authenticator);
    $url = "forum_index.php?read=1&$tokens&return=$return";
    show_button($url, "Mark all threads as read", "Mark all threads in this forum as 'read'.");
}

echo " <br><br></td>";
echo '    <form action="forum_forum.php" method="get">
    <input type="hidden" name="id" value="'.$forum->getID().'">';
echo '<td align="right">';
echo select_from_array("sort", $forum_sort_styles, $sort_style);
echo '<input type="submit" value="OK"><br><br></td>';
echo "</tr>\n</table>\n</form>";

show_forum($forum, $start, $sort_style, $user);

page_tail();

/**
 * This function shows the threadlist for the given forum
 * Starting from $start,
 * using the given $sort_style (as defined in forum.php)
 * and using the features for the logged in user in $user.
 **/
function show_forum($forum, $start, $sort_style, $user) {
    $gotoStr = "<div align=\"right\">".show_page_nav($forum,$start)."</div>";
    echo $gotoStr; // Display the navbar
    start_forum_table(array("", tr(FORUM_THREADS), tr(FORUM_POSTS), tr(FORUM_AUTHOR), tr(FORUM_VIEWS), "<nobr>".tr(FORUM_LAST_POST)."</nobr>"));
    
    $sticky_first = !$user || !$user->hasIgnoreStickyPosts();
    // Show hidden threads if logged in user is a moderator
    $show_hidden = $user && $user->isSpecialUser(S_MODERATOR); 
    $threads = $forum->getThreads($start, THREADS_PER_PAGE, $sort_style, $show_hidden, $sticky_first);
    
    // Run through the list of threads, displaying each of them
    $n = 0; $i=0;
    foreach ($threads as $key => $thread) {
        $owner = $thread->getOwner();
        $timestamp = $thread->getLastTimestamp();
        $unread = $user && ($timestamp>$thread->getLastReadTimestamp($user)) && ($timestamp > $user->getReadTimestamp());
        
        if ($thread->getStatus()==1){
            // This is an answered helpdesk thread
            echo '<tr class="row_hd'.$n.'">';
        } else {
            // Just a standard thread.
            echo '<tr class="row'.$n.'">';    
        }
        
        echo "<td width=\"1%\" align=\"right\"><nobr>";
        if ($user && ($thread->getRating()>$user->getHighRatingThreshold())) {
            show_image(EMPHASIZE_IMAGE, "This message has a high average rating");
        }
        if ($user && ($thread->getRating()<$user->getLowRatingThreshold())) {
            show_image(FILTER_IMAGE, "This message has a low average rating");
        }
        if ($thread->isHidden()) {
            echo "X";
        }
        if ($unread) {
            if ($thread->isSticky()) {
                if ($thread->isLocked()) {
                    show_image(NEW_IMAGE_STICKY_LOCKED, "This thread is sticky and locked, and you haven't read it yet");
                } else {
                    show_image(NEW_IMAGE_STICKY, "This thread is sticky and you haven't read it yet");
                }
            } else {
                if ($thread->isLocked()) {
                    show_image(NEW_IMAGE_LOCKED, "You haven't read this thread yet, and it's locked");
                } else {
                    show_image(NEW_IMAGE, "You haven't read this thread yet");
                }
            }
        } else {
            if ($thread->isSticky()) {
                if ($thread->isLocked()) {
                    show_image(IMAGE_STICKY_LOCKED, "This thread is sticky and locked");
                } else {
                    show_image(IMAGE_STICKY, "This thread is sticky");
                }
            } else {
                if ($thread->isLocked()) {
                    show_image(IMAGE_LOCKED, "This thread is locked");
                }
            }
        }
        echo "</nobr></td>";

        $titlelength = 48;
        $title = $thread->getTitle();
        if (strlen($title) > $titlelength) {
                $title = substr($title,0,$titlelength)."...";
        }
        $title = cleanup_title($title);
        echo '<td class="threadline"><a href="forum_thread.php?id='.$thread->getID().'"><b>'.$title.'</b></a><br></td>';
        $n = ($n+1)%2;

        echo '
            <td>'.($thread->getPostCount()+1).'</td>
            <td align="left"><div class="authorcol">'.re_user_links($owner).'</div></td>
            <td>'.$thread->getViewCount().'</td>
            <td style="text-align:right">'.time_diff_str($thread->getLastTimestamp(), time()).'</td>
            </tr>
        ';
        flush();
    }
    end_forum_table();
    echo $gotoStr; // Display the navigation bar at the bottom as well.
}


?>
