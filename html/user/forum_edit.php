<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

/**
 * Using this page you can edit a post.
 * First it displays a box to edit in, and when you submit the changes
 * it will call the methods on the post to make the changes.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');

db_init();

$logged_in_user = re_get_logged_in_user();

// if user is a project admin or project developer or forum moderator,
// allow them to edit their own posts indefinitely.
$is_spec = $logged_in_user->isSpecialUser(S_MODERATOR) ||
	   $logged_in_user->isSpecialUser(S_ADMIN) ||
	   $logged_in_user->isSpecialUser(S_DEV);

$postid = get_int("id");
$post = new Post($postid);
$thread = $post->getThread();

// Check some prerequisits for editting the post
if (!$is_spec && (time() > $post->getTimestamp() + MAXIMUM_EDIT_TIME)){
    error_page ("You can no longer edit this post.<br />Posts can only be edited at most ".(MAXIMUM_EDIT_TIME/60)." minutes after they have been created.");
}
$post_owner = $post->getOwner();
if (($logged_in_user->getID() != $post_owner->getID()) || (can_reply($thread, $logged_in_user) == false)) {
    error_page ("You are not authorized to edit this post.");
}

$thread_owner = $thread->getOwner();
$can_edit_title = ($post->getParentPostID()==0 and $thread_owner->getID()==$logged_in_user->getID());

$content = post_str("content", true);
$preview = post_str("preview", true);

if (post_str('submit',true) && (!$preview)) {    
    check_tokens($logged_in_user->getAuthenticator());
    
    if (post_str('add_signature', true) == "1"){
        $add_signature = true;
    }  else {
        $add_signature = false;
    }
    $post->setSignature($add_signature);
    
    $post->setContent($content);
    // If this post belongs to the creator of the thread and is at top-level 
    // (ie. not a response to another post) allow the user to modify the thread title
    if ($can_edit_title){
        $thread->setTitle(post_str('title'));
    }

    header('Location: forum_thread.php?id='.$thread->getID());
}


page_head('Forum');

$forum = $thread->getForum();
$category = $forum->getCategory();

show_forum_title($forum, $thread);

if ($preview == tra("Preview")) {
    $options = new output_options;
    echo "<div id=\"preview\">\n";
    echo "<div class=\"header\">".tra("Preview")."</div>\n";
    echo output_transform($content, $options);
    echo "</div>\n";
}

echo "<form action=\"forum_edit.php?id=".$post->getID()."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->getAuthenticator());
start_table();
row1("Edit your post");
if ($can_edit_title) {
    //If this is the user can edit the thread title display a way of doing so
    if ($preview) {
        row2(
            tr(FORUM_SUBMIT_NEW_TITLE).html_info(),
            "<input type=\"text\" name=\"title\" value=\"".stripslashes(htmlspecialchars($title))."\">"
        );
    } else {
        row2(
            tr(FORUM_SUBMIT_NEW_TITLE).html_info(),
            '<input type="text" name="title" value="'.stripslashes(htmlspecialchars($thread->getTitle())).'">'
        );
    }
};

if ($preview) {
    row2(
        tr(FORUM_MESSAGE).html_info().post_warning(),
        "<textarea name=\"content\" rows=\"12\" cols=\"80\">".stripslashes(htmlspecialchars($content))."</textarea>"
    );
} else {
    row2(
        tr(FORUM_MESSAGE).html_info().post_warning(),
        '<textarea name="content" rows="12" cols="80">'.stripslashes(htmlspecialchars($post->getContent())).'</textarea>'
    );
}

if ($post->hasSignature()) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}
row2("", "<input id=\"add_signature\" name=\"add_signature\" value=\"1\" ".$enable_signature." type=\"checkbox\">
    <label for=\"add_signature\">".tr(FORUM_ADD_MY_SIG)."</label>");
row2("", "<input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\"><input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

?>
