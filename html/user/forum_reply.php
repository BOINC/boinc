<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/subscribe.inc');

db_init();

$logged_in_user = get_logged_in_user(true);
$logged_in_user = getForumPreferences($logged_in_user);

$thread = getThread(get_int('thread'));
$forum = getForum($thread->forum);
$category = getCategory($forum->category);
$helpdesk = $category->is_helpdesk;

$sort_style = get_str('sort', true);
$filter = get_str('filter', true);

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

if (!$thread){
    error_page("No such thread found");
}

if ($thread->hidden) {
    //If the thread has been hidden, do not display it, or allow people to continue to post
    //to it.
    error_page(
        "This thread has been hidden for administrative purposes.");
}

if ($logged_in_user->total_credit<$forum->post_min_total_credit || $logged_in_user->expavg_credit<$forum->post_min_expavg_credit){
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(
        "In order to reply to a post in ".$forum->title." you must have a certain amount of credit.
        This is to prevent and protect against abuse of the system.");
}

if (time()-$logged_in_user->last_post<$forum->post_min_interval){
    //If the user is posting faster than forum regulations allow
    //Tell the user to wait a while before creating any more posts
    error_page(
        "You cannot reply to any more posts right now. Please wait a while before trying again.<br />
        This delay has been enforced to protect against abuse of the system.");
}

if ($category->is_helpdesk) {
    if (!$sort_style) {
        $sort_style = getSortStyle($logged_in_user,"answer");
    } else {
        setSortStyle($logged_in_user,"answer", $sort_style);
    }
    page_head($title);
} else {
    if (!$sort_style) {
        $sort_style = getSortStyle($logged_in_user,"thread");
    } else {
        setSortStyle($logged_in_user,"thread", $sort_style);
    }
}

if ($sort_style == NULL) {
    $sort_style = "timestamp";
}

if (!empty($_POST['content'])) {
    $thread_id = get_int('thread');

    if (!empty($_GET['post'])) {
        $parent_post = $_GET['post'];
    } else {
        $parent_post = NULL;
    }

    if ($_POST['add_signature']=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }

    replyToThread($thread_id, $logged_in_user->id, $_POST['content'], $parent_post, $add_signature);
    notify_subscribers($thread_id);
    header('Location: forum_thread.php?id='.$thread_id);
}


if (get_int('post', true)) {
    $post = getPost(get_int('post'));
}


// TODO: Write a function for this.
if ($helpdesk) {
    page_head('Questions and answers');
} else {
    page_head('Message boards');
}

show_forum_title($forum, $thread, $helpdesk);

start_forum_table(array("Author", "Message"));

show_message_row($thread, $category, $post);
show_posts($thread, $sort_style,-2, false, false, $helpdesk);

end_forum_table();

page_tail();

function show_message_row($thread, $category, $post=NULL) {
    global $logged_in_user;

    $x1 = "Message:".html_info().post_warning();
    $x2 = "";
    if ($post) {
        $x2 .=" reply to <a href=#$post->id>Message ID $post->id</a>:";
    }
    if ($category->is_helpdesk) {
        $x2 .= "<br><b>
            Please use this form ONLY to answer or
            discuss this particular question or problem.
        ";
    }
    $x2 .= "<form action=forum_reply.php?thread=$thread->id";

    if ($post) {
        $x2 .= "&post=$post->id";
    }

    $x2 .= " method=post><textarea name=content rows=18 cols=80>";
    if ($post) $x2 .= cleanTextBox(quote_text(stripslashes($post->content), 80));
    if ($logged_in_user->no_signature_by_default==0){
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
