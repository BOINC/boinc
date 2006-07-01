<?php
/**
 * Using this file you can post a reply to a thread.  Both input (form) and
 * action take place here.
 **/

require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');

db_init();

$logged_in_user = re_get_logged_in_user(true);

$thread = new Thread (get_int('thread'));
$forum = $thread->getForum();
$category = $forum->getCategory();

$sort_style = get_str('sort', true);
$filter = get_str('filter', true);
$content = post_str('content', true);
$parent_post_id = get_int('post', true);
if ($parent_post_id) $parent_post = new Post($parent_post_id);

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

if ($thread->isHidden()) {
    //If the thread has been hidden, do not display it, or allow people to continue to post
    //to it.
    error_page(
        "This thread has been hidden for administrative purposes.");
}

if ($logged_in_user->getTotalCredit()<$forum->getPostMinTotalCredit() || $logged_in_user->getExpavgCredit()<$forum->getPostMinExpavgCredit()){
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(
        "In order to reply to a post in ".$forum->title." you must have a certain amount of credit.
        This is to prevent and protect against abuse of the system.");
}

if (time()-$logged_in_user->getLastPostTimestamp()<$forum->getPostMinInterval()){
    //If the user is posting faster than forum regulations allow
    //Tell the user to wait a while before creating any more posts
    error_page(
        "You cannot reply to any more posts right now. Please wait a while before trying again.<br />
        This delay has been enforced to protect against abuse of the system.");
}

if (!$sort_style) {
    $sort_style = $logged_in_user->getThreadSortStyle("thread");
} else {
    $logged_in_user->setThreadSortStyle($sort_style);
}

if ($content){
    if (post_str('add_signature',true)=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }
    $thread->createReply($content, $parent_post, $logged_in_user, $add_signature);
    header('Location: forum_thread.php?id='.$thread->getID());
}

page_head(tr(FORUM_TITLE_SHORT));

show_forum_title($forum, $thread);
start_forum_table(array(tr(FORUM_AUTHOR), tr(FORUM_MESSAGE)));

show_message_row($thread, $parent_post);
show_posts($thread, $sort_style, $filter, $logged_in_user, true);
end_forum_table();

page_tail();

function show_message_row($thread, $parent_post) {
    global $logged_in_user;

    $x1 = "Message:".html_info().post_warning();
    $x2 = "";
    if ($parent_post) {
        $x2 .=" reply to <a href=#".$parent_post->getID().">Message ID ".$parent_post->getID()."</a>:";
    }
    $x2 .= "<form action=forum_reply.php?thread=".$thread->getID();

    if ($parent_post) {
        $x2 .= "&post=".$parent_post->getID();
    }

    $x2 .= " method=post><textarea name=content rows=18 cols=80>";

    if ($parent_post) $x2 .= cleanTextBox(quote_text(stripslashes($parent_post->getContent()), 80));
    if ($logged_in_user->hasSignatureByDefault()){
        $enable_signature="checked=\"true\"";
    } else {
        $enable_signature="";
    }
    $x2 .= "\n</textarea><p>
        <input type=submit value=\"Post reply\">
        &nbsp;&nbsp;&nbsp;
        <input name=add_signature value=add_it ".$enable_signature." type=checkbox>Add my signature to this reply                                

        </form>
    ";
    row2($x1, $x2);
}

function quote_text($text, $cols = 0) {
	/* $cols is depricated. */
    $text = "[quote]" . $text . "[/quote]";
    return $text;
}
?>
