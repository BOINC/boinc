<?php
// display the contents of a thread.

require_once('../inc/util.inc');
require_once('../inc/forum.inc');

$threadid = get_int('id');
$sort_style = get_int('sort', true);
$filter = get_str('filter', true);

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

$logged_in_user = get_logged_in_user(false);
$tokens = "";
if ($logged_in_user) {
    BoincForumPrefs::lookup($logged_in_user);
    $tokens = url_tokens($logged_in_user->authenticator);
}

$thread = BoincThread::lookup_id($threadid);
$forum = BoincForum::lookup_id($thread->forum);

if ($thread->hidden) {
    if (!is_moderator($logged_in_user, $forum)) {
        error_page(
            tra("This thread has been hidden for administrative purposes")
        );
    }
}

$title = cleanup_title($thread->title);
if (!$sort_style) {
    // get the sorting style from the user or a cookie
    if ($logged_in_user){
        $sort_style = $logged_in_user->prefs->thread_sorting;
    } else {
        list($forum_style, $sort_style)=explode("|",$_COOKIE['sorting']);
    }
} else {
    if ($logged_in_user){
        $logged_in_user->prefs->thread_sorting = $sort_style;
        $logged_in_user->prefs->update("thread_sorting=$sort_style");
    } else {
        list($forum_style,$old_style)=explode("|",$_COOKIE['sorting']);
        setcookie('sorting', implode("|",array($forum_style,$sort_style)), time()+3600*24*365);
    }
}

if ($logged_in_user && $logged_in_user->prefs->jump_to_unread){
    page_head($title, 'jumpToUnread();');
    echo "<link href=\"forum_forum.php?id=".$forum->id."\" rel=\"up\" title=\"".$forum->title."\">";
} else {
    page_head($title);
    echo "<link href=\"forum_forum.php?id=".$forum->id."\" rel=\"up\" title=\"".$forum->title."\">";
}

$is_subscribed = $logged_in_user && BoincSubscription::lookup($logged_in_user->id, $thread->id);

show_forum_header($logged_in_user);
if ($forum->parent_type == 0) {
    $category = BoincCategory::lookup_id($forum->category);
    show_forum_title($category, $forum, $thread);

    if ($category->is_helpdesk && !$thread->status){
        if ($logged_in_user){
            if ($thread->owner == $logged_in_user->id){
                if ($thread->replies !=0) {
                    // Show a "this question has been answered" to the author
                    echo "<div class=\"helpdesk_note\">
                    <form action=\"forum_thread_status.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->id."\">
                    <input type=\"submit\" value=\"My question was answered\">
                    </form>
                    If your question has been adequately answered please click here to close it!
                    </div>";
                }
            } else {
                // and a "I also got this question" to everyone else if they havent already told so
                echo "<div class=\"helpdesk_note\">
                <form action=\"forum_thread_vote.php\"><input type=\"hidden\" name=\"id\" value=\"".$thread->id."\">
                <input type=\"submit\" value=\"I've also got this question\">
                </form>
                </div>";
            }
        }
    }
}

echo "
    <table width=\"100%\" cellspacing=0 cellpadding=0>
    <tr>
    <td align=\"left\">
";

$reply_url = "";
if (can_reply($thread, $forum, $logged_in_user)) {        
    $reply_url = "forum_reply.php?thread=".$thread->id."#input";
    show_button($reply_url, tra("Post to thread"), "Add a new message to this thread");
}

if ($is_subscribed) {
    $url = "forum_subscribe.php?action=unsubscribe&thread=".$thread->id."$tokens";
    show_button($url, tra("Unsubscribe"), "You are subscribed to this thread.  Click here to unsubscribe.");
} else {
    $url = "forum_subscribe.php?action=subscribe&thread=".$thread->id."$tokens";
    show_button($url, tra("Subscribe"), "Click to get email when there are new posts in this thread");
}

//If the logged in user is moderator enable some extra features
//
if (is_moderator($logged_in_user, $forum)) {
    if ($thread->hidden){
        show_button("forum_moderate_thread_action.php?action=unhide&thread=".$thread->id."$tokens", "Unhide", "Unhide this thread");
    } else {
        show_button("forum_moderate_thread.php?action=hide&thread=".$thread->id, "Hide", "Hide this thread");
    }
    if ($thread->sticky){
        show_button("forum_moderate_thread_action.php?action=desticky&thread=".$thread->id."$tokens", "Make unsticky", "Make this thread not sticky");
    } else {
        show_button("forum_moderate_thread_action.php?action=sticky&thread=".$thread->id."$tokens", "Make sticky", "Make this thread sticky");
    }
    if ($thread->locked) {
        show_button("forum_moderate_thread_action.php?action=unlock&amp;thread=".$thread->id."$tokens", "Unlock", "Unlock this thread");
    } else {
        show_button("forum_moderate_thread_action.php?action=lock&thread=".$thread->id."$tokens", "Lock", "Lock this thread");
    }
    if ($forum->parent_type == 0) {
        show_button("forum_moderate_thread.php?action=move&thread=".$thread->id."$tokens", "Move", "Move this thread to a different forum");
    }
    show_button("forum_moderate_thread.php?action=title&thread=".$thread->id."$tokens", "Edit title", "Edit thread title");
}

// Display a box that allows the user to select sorting of the posts
echo "</td><td align=right style=\"border:0px\">
    <form action=\"forum_thread.php\">
    <input type=\"hidden\" name=\"id\" value=\"", $thread->id, "\">
    Sort 
";
echo select_from_array("sort", $thread_sort_styles, $sort_style);
echo "<input type=submit value=Sort>
    </form>
    </td></tr></table>
";

// Here is where the actual thread begins.
$headings = array(array(tra("Author"),"authorcol"), array(tra("Message"),"",2));

start_forum_table($headings, "id=\"thread\" width=100%");
show_posts($thread, $forum, $sort_style, $filter, $logged_in_user, true);
end_table();

if ($reply_url) {
    show_button($reply_url, tra("Post to thread"), "Add a new message to this thread");
}

if ($forum->parent_type == 0) {
    show_forum_title($category, $forum, $thread);
}

$thread->update("views=views+1");

page_tail();
$cvs_version_tracker[]="\$Id$";
?>
