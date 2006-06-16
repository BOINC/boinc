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
	list($sort_style,$thread_style)=explode("|",$_COOKIE['sorting']);    
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

page_head(tr(FORUM_TITLE_SHORT).' : '.$forum->getTitle());
// Allow users with a linktab-browser to get some usefull links
echo '<link href="forum_index.php" rel="up" title="Forum Index">';

echo '
    <table width="100%" cellspacing="0" cellpadding="0">
    <tr valign="bottom">
    <td colspan=2>';

show_forum_title($forum, NULL);

$Category = $forum->getCategory(); 
if ($Category->getType()!=0){
    echo "
    <div class=\"helpdesk_note\">
	<br /><form action=\"forum_search.php\"><input type=\"hidden\" name=\"forumid\" value=\"".$forum->getID()."\"><input type=\"submit\" value=\"Search ".$forum->getTitle()."\"></form>
	To keep the number of repeated posts to a minimum please <a href=\"forum_search.php\"><b><font color=\"green\">search</font></b></a> before you create a new thread.
    </div></td></tr><tr><td>";
}


echo '<p><a href="forum_post.php?id='.$id.'">';
echo "[Create a new thread]</a></p></td>";
echo '    <form action="forum_forum.php" method="get">
    <input type="hidden" name="id" value="'.$forum->getID().'">';
echo '<td align="right">';
show_select_from_array("sort", $forum_sort_styles, $sort_style);
echo '<input type="submit" value="OK"></td>';
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
        $unread = $user && ($thread->getLastTimestamp()>$thread->getLastReadTimestamp($user));

        if ($thread->getStatus()==1){
	    // This is an answered helpdesk thread
	    echo '<tr class="row_hd'.$n.'">';
	} else {
	    // Just a standard thread.
	    echo '<tr class="row'.$n.'">';	
	}

        echo "<td width=\"1%\" align=\"right\"><nobr>";
        if ($user && ($thread->getRating()>$user->getHighRatingThreshold())) {
            echo "<img src=\"".EMPHASIZE_IMAGE."\" alt=\"Emphasized thread\">";
        }
        if ($user && ($thread->getRating()<$user->getLowRatingThreshold())) {
            echo "<img src=\"".FILTER_IMAGE."\" alt=\"Filtered thread\">";
        }
        if ($thread->isHidden()) {
            echo "X";
        }
        if ($unread && !$thread->isSticky()) {
            echo "<img src=\"".NEW_IMAGE."\" alt=\"Unread post(s)\">";
        }
        elseif($unread) {
            echo "<img src=\"".NEW_IMAGE_STICKY."\" alt=\"Unread post(s)\">";
        }
        elseif($thread->isSticky()) {
            echo "<img src=\"".STICKY_IMAGE."\" alt=\"Sticky\">";
        }
        echo "</nobr></td>";

        $titlelength = 48;
        $title = cleanup_title($thread->getTitle());
        if (strlen($title) > $titlelength) {
                $title = substr($title,0,$titlelength)."...";
        }
        echo '<td class="threadline"><a href="forum_thread.php?id='.$thread->getID().'"><b>'.$title.'</b></a><br></td>';
        $n = ($n+1)%2;

        echo '
                <td>'.($thread->getPostCount()+1).'</td>
                <td align="left"><div class="authorcol">'.re_user_links($owner).'</div></td>
                <td>'.$thread->getViewCount().'</td>
                <td style="text-align:right">'.time_diff_str($thread->getLastTimestamp(), time()).'</td>
	    </tr>';
	flush();
    }
    end_forum_table();
    echo $gotoStr; // Display the navigation bar at the bottom as well.
}


?>
