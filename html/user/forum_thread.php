<?php
/**
 * This file displays the contents of a thread.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
db_init();

$threadid = get_int('id');
$sort_style = get_int('sort', true);
$filter = get_str('filter', true);

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

// Fetch the thread and increment the number of views
$thread = new Thread($threadid);

$forum = $thread->getForum();
$category = $forum->getCategory();
$logged_in_user = re_get_logged_in_user(false);

$title = cleanup_title($thread->getTitle());
if (!$sort_style) {
    // get the sorting style from the user or a cookie
    if ($logged_in_user){
	$sort_style = $logged_in_user->getThreadSortStyle();
    } else {
        list($forum_style, $sort_style)=explode("|",$_COOKIE['sorting']);
    }
} else {
    if ($logged_in_user){
	$logged_in_user->setThreadSortStyle($sort_style);
    } else {
        list($forum_style,$old_style)=explode("|",$_COOKIE['sorting']);
        setcookie('sorting', implode("|",array($forum_style,$sort_style)), time()+3600*24*365);
    }	
}



if ($logged_in_user && $logged_in_user->hasJumpToUnread()){
    page_head($title, 'jumpToUnread();');
    echo "<link href=\"forum_forum.php?id=".$forum->id."\" rel=\"up\" title=\"".$forum->getTitle()."\">";
} else {
    page_head($title);
    echo "<link href=\"forum_forum.php?id=".$forum->id."\" rel=\"up\" title=\"".$forum->getTitle()."\">";
}

$is_subscribed = $logged_in_user && $thread->isSubscribed($logged_in_user);

show_forum_title($forum, $thread);
if (($thread->isHidden()) && $logged_in_user && (!$logged_in_user->isSpecialUser(S_MODERATOR))) {
    /* If the user logged in is a moderator, show him the
    * thread if he goes so far as to name it by ID like this.
    * Otherwise, hide the thread.
    */
    error_page(tr(FORUM_THREAD_HIDDEN));
} else {    

    if ($thread->getType()!=0 && $thread->getStatus()==0){
        $thread_owner = $thread->getOwner();
	if ($logged_in_user){
	    if ($thread_owner->getID() == $logged_in_user->getID()){
	        if ($thread->getPostCount()==0){} else {
		    // Show a "this question has been answered" to the author
		    echo "<div class=\"helpdesk_note\">
		    <form action=\"forum_thread_status.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->getID()."\">
		    <input type=\"submit\" value=\"My question was answered\">
		    </form>
		    If your question has been adequately answered please click here to close it!
		    </div>";
	        }
	    } else {
	        // and a "I also got this question" to everyone else if they havent already told so
	        echo "<div class=\"helpdesk_note\">
	        <form action=\"forum_thread_vote.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->getID()."\">
	        <input type=\"submit\" value=\"I've also got this question\">
	        </form>
	        </div>";
	    }
	}
    }

    echo "
        <form action=\"forum_thread.php\">
        <input type=\"hidden\" name=\"id\" value=\"", $thread->getID(), "\">
        <table width=\"100%\" cellspacing=0 cellpadding=0>
        <tr>
        <td align=\"left\">";
    echo $reply_text = "<a href=\"forum_reply.php?thread=".$thread->getID()."#input\">".tr(FORUM_THREAD_REPLY)."</a><br>";

    if ($is_subscribed) {
        echo tr(FORUM_THREAD_SUBSCRIBED)." ";
        echo "<a href=\"forum_subscribe.php?action=unsubscribe&amp;thread=".$thread->getID()."\">".tr(FORUM_THREAD_UNSUBSCRIBE)."</a>.";
    } else {
        echo "<a href=\"forum_subscribe.php?action=subscribe&amp;thread=".$thread->getID()."\">".tr(FORUM_THREAD_SUBSCRIBE)."</a>";
    }

    //If the logged in user is moderator enable some extra features
    if ($logged_in_user && $logged_in_user->isSpecialUser(S_MODERATOR)){
	if ($thread->isHidden()){
	    echo "<br /><a href=\"forum_moderate_thread_action.php?action=unhide&amp;thread=".$thread->getID()."\">Un-Delete this thread</a>";
	} else {
	    echo "<br /><a href=\"forum_moderate_thread.php?action=hide&amp;thread=".$thread->getID()."\">Delete this thread</a>";
	}
	if($thread->isSticky()){
	    echo "<br /><a href=\"forum_moderate_thread_action.php?action=desticky&amp;thread=".$thread->getID()."\">De-sticky this thread</a>"; 
	} else {
	    echo "<br /><a href=\"forum_moderate_thread_action.php?action=sticky&amp;thread=".$thread->getID()."\">Make this thread sticky</a>";
	}	
        echo "<br /><a href=\"forum_moderate_thread.php?action=move&amp;thread=".$thread->getID()."\">Move this thread</a>";
        echo "<br /><a href=\"forum_moderate_thread.php?action=title&amp;thread=".$thread->getID()."\">Edit thread title</a>";
    }

    // Display a box that allows the user to select sorting of the posts
    echo "</td><td align=right style=\"border:0px\">";
    echo "Sort ";
    show_select_from_array("sort", $thread_sort_styles, $sort_style);
    echo "<input type=submit value=OK>\n</td>";
    echo "</tr>\n</table>\n</form>\n";

    // Here is where the actual thread begins.
    $headings = array(array(tr(FORUM_AUTHOR),"authorcol"), array(tr(FORUM_MESSAGE),"",2));

    start_forum_table($headings, "id=\"thread\" width=100%");
    show_posts($thread, $sort_style, $filter, $logged_in_user, true);
    end_forum_table();

    echo "<p>".$reply_text;
    show_forum_title($forum, $thread);
    $thread->incViews();

}

page_tail();
?>
