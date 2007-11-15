<?php

// This file allows you to create a new thread in a forum
// At first it displays an input box and when you submit
// it will apply the changes by calling methods on the forum

require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/akismet.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

check_banished($logged_in_user);

$forumid = get_int("id");
$forum = BoincForum::lookup_id($forumid);

if ($forum->is_dev_blog){
    if (
       (!$logged_in_user->prefs->privilege(S_SCIENTIST)) &&
       (!$logged_in_user->prefs->privilege(S_DEV)) &&
       (!$logged_in_user->prefs->privilege(S_ADMIN))
       ) {
         // Since this is a devBlog only people at the project can start threads here.
         error_page("This forum is marked as a development blog, only people directly working with the project may start a new thread here. <br/>However, you may post a reply to an existing thread.");
    }
}

if (!$logged_in_user->prefs->privilege(S_MODERATOR) && ($logged_in_user->total_credit<$forum->post_min_total_credit || $logged_in_user->expavg_credit<$forum->post_min_expavg_credit)) {
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(tra("In order to create a new thread in %1 you must have a certain amount of credit. This is to prevent and protect against abuse of the system.", $forum->title));
}
if (time()-$logged_in_user->last_post <$forum->min_post_interval){
    //If the user is posting faster than forum regulations allow
    //Tell the user to wait a while before creating any more posts
    error_page(tra("You cannot create any more threads right now. Please wait a while before trying again. This delay has been enforced to protect against abuse of the system."));
}
$title = post_str("title", true);
$content = post_str("content", true);
$preview = post_str("preview", true);

if ($content && $title && (!$preview)){
    if (post_str('add_signature',true)=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }
    check_tokens($logged_in_user->authenticator);
    akismet_check($logged_in_user, $content);
    $thread = create_thread(
        $title, $content, $logged_in_user, $forum, $add_signature
    );
    header('Location: forum_thread.php?id=' . $thread->id);
}

page_head('Create new thread');

$category = BoincCategory::lookup_id($forum->category);
show_forum_header($logged_in_user);
show_forum_title($category, $forum, null);

if ($preview == tra("Preview")) {
    $options = null;
    echo "<div id=\"preview\">\n";
    echo "<div class=\"header\">".tra("Preview")."</div>\n";
    echo output_transform($content, $options);
    echo "</div>\n";
}

echo "<form action=\"forum_post.php?id=".$forum->id."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->authenticator);

start_table();

row1(tra("Create a new thread")); //New thread
$submit_help = "";
$body_help = "";

//Title
if ($content && !$title) $submit_help = "<br /><font color=\"red\">Remember to add a title</font>";
row2(tra("Title").$submit_help, "<input type=\"text\" name=\"title\" size=\"62\" value=\"".stripslashes(htmlspecialchars($title))."\">");
//Message
row2(tra("Message").html_info().post_warning().$body_help, "<textarea name=\"content\" rows=\"12\" cols=\"54\">".stripslashes(htmlspecialchars($content))."</textarea>");

if (!$logged_in_user->prefs->no_signature_by_default) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}

row2("", "<input name=\"add_signature\" value=\"add_it\" ".$enable_signature." type=\"checkbox\">".tra("Add my signature to this post"));
row2("", "<input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\"> <input type=\"submit\" value=\"OK\">");


end_table();

echo "</form>\n";

page_tail();

$cvs_version_tracker[]="\$Id$";
?>
