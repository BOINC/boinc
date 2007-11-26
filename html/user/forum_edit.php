<?php

// Using this page you can edit a post.
// First it displays a box to edit in, and when you submit the changes
// it will call the methods on the post to make the changes.
//

require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

$postid = get_int("id");
$post = BoincPost::lookup_id($postid);
if (!$post) error_page("no such post");
$thread = BoincThread::lookup_id($post->thread);
if (!$thread) error_page("no such thread");
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($logged_in_user, $forum)) {
    if (time() > $post->timestamp + MAXIMUM_EDIT_TIME) {
        error_page(
            "You can no longer edit this post.<br />Posts can only be edited at most ".(MAXIMUM_EDIT_TIME/60)." minutes after they have been created."
        );
    }
}

$post_owner = BoincUser::lookup_id($post->user);
if (($logged_in_user->id != $post_owner->id) || (can_reply($thread, $forum, $logged_in_user) == false)) {
    error_page ("You are not authorized to edit this post.");
}

$thread_owner = BoincUser::lookup_id($thread->owner);

// If this post belongs to the creator of the thread and is at top-level 
// (ie. not a response to another post)
// allow the user to modify the thread title
//
$can_edit_title = ($post->parent_post==0 and $thread_owner->id==$logged_in_user->id);

$content = post_str("content", true);
$title = post_str("title", true);
$preview = post_str("preview", true);

if (post_str('submit',true) && (!$preview)) {
    check_tokens($logged_in_user->authenticator);
    
    $add_signature = (post_str('add_signature', true) == "1")?1:0;
    $content = substr($content, 0, 64000);
    $content = trim($content);
    if (strlen($content)) {
        $content = BoincDb::escape_string($content);
        $now = time();
        $post->update("signature=$add_signature, content='$content', modified=$now");
    
        if ($can_edit_title){
            $title = trim($title);
            $title = strip_tags($title);
            $title = BoincDb::escape_string($title);
            $thread->update("title='$title'");
        }
        header("Location: forum_thread.php?id=$thread->id");
    } else {
        delete_post($post, $thread, $forum);
        header("Location: forum_forum.php?id=$forum->id");
    }
}

page_head('Forum');

show_forum_header($logged_in_user);
switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category);
    show_forum_title($category, $forum, $thread);
    break;
case 1:
    show_team_forum_title($forum, $thread);
    break;
}

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
row1("Edit your message");
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
