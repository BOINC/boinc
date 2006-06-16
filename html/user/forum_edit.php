<?php
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
if ($logged_in_user->getID() != $post_owner->getID()) {
    error_page ("You are not authorized to edit this post.");
}

$thread_owner = $thread->getOwner();
$can_edit_title = ($post->getParentPostID()==0 and $thread_owner->getID()==$logged_in_user->getID());

if (post_str('submit',true)) {    
    
    $post->setContent(post_str('content'));
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

echo "<form action=\"forum_edit.php?id=".$post->getID()."\" method=\"POST\">\n";

start_table();
row1("Edit your post");
if ($can_edit_title) {
    //If this is the user can edit the thread title display a way of doing so
    row2(
	    tr(FORUM_SUBMIT_NEW_TITLE).html_info(),
	    "<input type=text name=title value=\"".stripslashes($thread->getTitle())."\">"
    );
};

row2(
    tr(FORUM_MESSAGE).html_info().post_warning(),
    "<textarea name=\"content\" rows=12 cols=80>".cleanTextBox(stripslashes($post->getContent()))."</textarea>"
);
row2(
    "",
    "<input type=submit name=submit value=OK>"
);

end_table();

echo "</form>";

page_tail();

?>
