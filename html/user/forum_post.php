<?php
/**
 * This file allows you to create a new thread in a forum
 * At first it displays an input box and when you submit
 * it will apply the changes by calling methods on the forum
 **/
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit 
require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
require_once('../inc/akismet.inc');

db_init();

$logged_in_user = re_get_logged_in_user(true);

check_banished($logged_in_user);

$forumid = get_int("id");
$forum = new Forum($forumid);


if ($forum->isDevBlog()){
    if (
       (!$logged_in_user->isSpecialUser(S_SCIENTIST)) &&
       (!$logged_in_user->isSpecialUser(S_DEV)) &&
       (!$logged_in_user->isSpecialUser(S_ADMIN))
       ) {
         // Since this is a devBlog only people at the project can start threads here.
         error_page("This forum is marked as a development blog, only people directly working with the project may start a new thread here. <br/>However, you may post a reply to an existing thread.");
    }
}

if (!$logged_in_user->isSpecialUser(S_MODERATOR) && ($logged_in_user->getTotalCredit()<$forum->getPostMinTotalCredit() || $logged_in_user->getExpavgCredit()<$forum->getPostMinExpavgCredit())) {
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(sprintf(tr(FORUM_ERR_EXPAVG),$forum->getTitle()));
}
if (time()-$logged_in_user->getLastPostTimestamp()<$forum->getPostMinInterval()){
    //If the user is posting faster than forum regulations allow
    //Tell the user to wait a while before creating any more posts
    error_page(tr(FORUM_ERR_INTERVAL));
}
$title = post_str("title", true);
$content = post_str("content", true);

if ($content && $title){
    if (post_str('add_signature',true)=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }
    check_tokens($logged_in_user->getAuthenticator());
    akismet_check($logged_in_user, $content);
    $thread = $forum->createThread($title, $content, $logged_in_user, $add_signature);
    header('Location: forum_thread.php?id=' . $thread->getID());
}

page_head('Forum');

show_forum_title($forum, NULL, $category->is_helpdesk);

echo "<form action=\"forum_post.php?id=".$forum->getID()."\" method=POST>\n";
echo form_tokens($logged_in_user->getAuthenticator());

start_table();

row1(tr(FORUM_SUBMIT_NEW)); //New thread
$submit_help = "";
$body_help = "";

//Title
if ($content && !$title) $submit_help = "<br /><font color=\"red\">Remember to add a title</font>";
row2(tr(FORUM_SUBMIT_NEW_TITLE).$submit_help, "<input type=\"text\" name=\"title\" size=\"62\">");
//Message
row2(tr(FORUM_MESSAGE).html_info().post_warning().$body_help, "<textarea name=\"content\" rows=\"12\" cols=\"54\">".stripslashes($content)."</textarea>");

if ($logged_in_user->hasSignatureByDefault()) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}

row2("", "<input name=\"add_signature\" value=\"add_it\" ".$enable_signature." type=\"checkbox\">".tr(FORUM_ADD_MY_SIG));
row2("", "<input type=\"submit\" value=\"OK\">");


end_forum_table();

echo "</form>\n";

page_tail();
?>
