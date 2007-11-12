<?php

// Using this page you can edit a post.
// First it displays a box to edit in, and when you submit the changes
// it will call the methods on the post to make the changes.
//

require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

// if user is a project admin or project developer or forum moderator,
// allow them to edit their own posts indefinitely.
//
$is_spec = $logged_in_user->prefs->privilege(S_MODERATOR) ||
	   $logged_in_user->prefs->privilege(S_ADMIN) ||
	   $logged_in_user->prefs->privilege(S_DEV);

$postid = get_int("id");
$post = BoincPost::lookup_id($postid);
$thread = BoincThread::lookup_id($post->thread);

// Check some prerequisites for editing the post
//
if (!$is_spec && (time() > $post->timestamp + MAXIMUM_EDIT_TIME)){
    error_page ("You can no longer edit this post.<br />Posts can only be edited at most ".(MAXIMUM_EDIT_TIME/60)." minutes after they have been created.");
}

$post_owner = BoincUser::lookup_id($post->user);
if (($logged_in_user->id != $post_owner->id) || (can_reply($thread, $logged_in_user) == false)) {
    error_page ("You are not authorized to edit this post.");
}

$thread_owner = BoincUser::lookup_id($thread->owner);
$can_edit_title = ($post->parent_post==0 and $thread_owner->id==$logged_in_user->id);

$content = post_str("content", true);
$preview = post_str("preview", true);

if (post_str('submit',true) && (!$preview)) {
    check_tokens($logged_in_user->authenticator);
    
    if (post_str('add_signature', true) == "1") {
        $add_signature = 1;
    }  else {
        $add_signature = 0;
    }
    $content = substr($content, 0, 64000);
    $content = BoincDb::escape_string($content);
    $post->update("signature=$add_signature, content='$content'");
    
    // If this post belongs to the creator of the thread and is at top-level 
    // (ie. not a response to another post)
    // allow the user to modify the thread title
    //
    if ($can_edit_title){
        $t = post_str('title');
        $t = trim($t);
        $t = strip_tags($ts);
        $t = BoincDb::escape_string($t);
        $thread->update("title='$t'");
    }

    header("Location: forum_thread.php?id=$thread->id");
}

page_head('Forum');

$forum = BoincForum::lookup_id($thread->forum);
$category = BoincCategory::lookup_id($forum->category);

show_forum_title($logged_in_user, $category, $forum, $thread);

if ($preview == tra("Preview")) {
    $options = null;
    echo "<div id=\"preview\">\n";
    echo "<div class=\"header\">".tra("Preview")."</div>\n";
    echo output_transform($content, $options);
    echo "</div>\n";
}

echo "<form action=\"forum_edit.php?id=".$post->id."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->authenticator);
start_table();
row1("Edit your post");
if ($can_edit_title) {
    //If this is the user can edit the thread title display a way of doing so
    if ($preview) {
        row2(
            tra("Title").html_info(),
            "<input type=\"text\" name=\"title\" value=\"".stripslashes(htmlspecialchars($title))."\">"
        );
    } else {
        row2(
            tra("Title").html_info(),
            '<input type="text" name="title" value="'.stripslashes(htmlspecialchars($thread->title)).'">'
        );
    }
};

if ($preview) {
    row2(
        tra("Message").html_info().post_warning(),
        "<textarea name=\"content\" rows=\"12\" cols=\"80\">".stripslashes(htmlspecialchars($content))."</textarea>"
    );
} else {
    row2(
        tra("Message").html_info().post_warning(),
        '<textarea name="content" rows="12" cols="80">'.stripslashes(htmlspecialchars($post->content)).'</textarea>'
    );
}

if ($post->signature) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}
row2("", "<input id=\"add_signature\" name=\"add_signature\" value=\"1\" ".$enable_signature." type=\"checkbox\">
    <label for=\"add_signature\">".tra("Add my signature to this post")."</label>");
row2("", "<input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\"><input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
