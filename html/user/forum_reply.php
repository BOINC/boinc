<?php

// Use this file you can post a reply to a thread.
// Both input (form) and action take place here.

require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/akismet.inc');

$logged_in_user = get_logged_in_user(true);
BoincForumPrefs::lookup($logged_in_user);
check_banished($logged_in_user);

$thread = BoincThread::lookup_id(get_int('thread'));

if ($thread->locked && !$logged_in_user->prefs->privilege(S_MODERATOR)
    && !$logged_in_user->prefs->privilege(S_ADMIN)) {
    error_page("This thread is locked. Only forum moderators and administrators are allowed to post there.");
}

$forum = BoincForum::lookup_id($thread->forum);
$category = BoincCategory::lookup_id($forum->category);

$sort_style = get_str('sort', true);
$filter = get_str('filter', true);
$content = post_str('content', true);
$preview = post_str("preview", true);
$parent_post_id = get_int('post', true);
$parent_post = null;
if ($parent_post_id) {
    $parent_post = BoincPost::lookup_id($parent_post_id);
    if ($parent_post->thread != $thread->id) {
        error_page("wrong thread");
    }
} else {
    $parent_post_id = 0;
}

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

if ($thread->hidden) {
    //If the thread has been hidden, do not display it, or allow people to continue to post
    //to it.
    error_page(
       "This thread has been hidden for administrative purposes."
    );
}

if (!$logged_in_user->prefs->privilege(S_MODERATOR) && ($logged_in_user->total_credit<$forum->post_min_total_credit || $logged_in_user->expavg_credit<$forum->post_min_expavg_credit)) {
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(
       "In order to reply to a post in ".$forum->title." you must have a certain amount of credit.
       This is to prevent and protect against abuse of the system."
    );
}

if (time()-$logged_in_user->prefs->last_post <$forum->post_min_interval){
    // If the user is posting faster than forum regulations allow
    // Tell the user to wait a while before creating any more posts
    error_page(
        "You cannot reply to any more posts right now. Please wait a while before trying again.<br />
        This delay has been enforced to protect against abuse of the system."
    );
}

if (!$sort_style) {
    $sort_style = $logged_in_user->prefs->thread_sorting;
} else {
    $logged_in_user->prefs->update("thread_sorting=$sort_style");
}

if ($content && (!$preview)){
    if (post_str('add_signature',true)=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }
    check_tokens($logged_in_user->authenticator);
    akismet_check($logged_in_user, $content);
    create_post($content, $parent_post_id, $logged_in_user, $forum, $thread, $add_signature);
    header('Location: forum_thread.php?id='.$thread->id);
}

page_head(tra("Message boards"));

show_forum_header($logged_in_user);
show_forum_title($category, $forum, $thread);

if ($preview == tra("Preview")) {
    $options = new output_options;
    echo "<div id=\"preview\">\n";
    echo "<div class=\"header\">".tra("Preview")."</div>\n";
    echo output_transform($content, $options);
    echo "</div>\n";
}

start_forum_table(array(tra("Author"), tra("Message")));

show_message_row($thread, $parent_post);
show_posts($thread, $sort_style, $filter, $logged_in_user, true);
end_table();

page_tail();

function show_message_row($thread, $parent_post) {
    global $logged_in_user;
    global $content;
    global $preview;

    $x1 = "Message:".html_info().post_warning();
    $x2 = "";
    if ($parent_post) {
        $x2 .=" reply to <a href=#".$parent_post->id.">Message ID ".$parent_post->id."</a>:";
    }
    $x2 .= "<form action=forum_reply.php?thread=".$thread->id;

    if ($parent_post) {
        $x2 .= "&post=".$parent_post->id;
    }

    $x2 .= " method=\"post\">\n";
    $x2 .= form_tokens($logged_in_user->authenticator);
    $x2 .= "<textarea name=\"content\" rows=\"18\" cols=\"80\">";
    if ($preview) {
        $x2 .= stripslashes(htmlspecialchars($content));
    } else {
        if ($parent_post) $x2 .= quote_text(stripslashes(htmlspecialchars($parent_post->content)), 80)."\n";
    }
    if (!$logged_in_user->prefs->no_signature_by_default){
        $enable_signature="checked=\"true\"";
    } else {
        $enable_signature="";
    }
    $x2 .= "</textarea><p>
        <input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\">
        <input type=\"submit\" value=\"Post reply\">
        &nbsp;&nbsp;&nbsp;
        <input name=\"add_signature\" id=\"add_signature\" value=\"add_it\" ".$enable_signature." type=\"checkbox\">
        <label for=\"add_signature\">Add my signature to this reply</label>

        </form>
    ";
    row2($x1, $x2);
}

function quote_text($text, $cols = 0) {
	/* $cols is depricated. */
    $text = "[quote]" . $text . "[/quote]";
    return $text;
}
$cvs_version_tracker[]="\$Id$";
?>
