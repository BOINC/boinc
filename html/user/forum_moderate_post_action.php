<?php

require_once("../inc/db.inc");
require_once("../inc/user.inc");
require_once("../inc/profile.inc");
require_once("../inc/util.inc");
require_once("../inc/image.inc");
require_once("../inc/forum.inc");


db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);
if (!isSpecialUser($user,0)) {
    // Can't moderate without being moderator
    echo "You are not authorized to moderate this post.";
    exit();
}

// TODO:  Write a request_str function to prevent stuff like this
if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page("You must specify an action...");
    } else {
	$action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$post = getPost(get_int('id'));
if (!$post) {
    // TODO: Standard error page
    echo "Invalid post ID.<br>";
    exit();
}

$thread = getThread($post->thread);

if ($action=="hide"){
    $result=mysql_query("update post set hidden = ".post_int("category")." where id=".$post->id);
    echo mysql_error();
} elseif ($action=="unhide"){
    $result=mysql_query("update post set hidden = 0 where id=".$post->id);
    echo mysql_error();
} elseif ($action=="move"){
    if (getThread(post_int('threadid'))){
        $result=mysql_query("update post set thread = ".post_int('threadid')." where id=".$post->id);
        echo mysql_error();
        //TODO: correct the number of posts in this thread
        //TODO: correct the number of posts in destination thread
    } else {
        echo "Destination not found, please check and try again.";
        exit();
    }
} else {
    echo "Unknown action ";
    exit();
}


if ($result) {
    echo mysql_error();
    if (post_str('reason', true)){
        send_moderation_email(lookup_user_id($post->user),$thread, $post, post_str("reason"));
    }
    header('Location: forum_thread.php?id='.$thread->id);
} else {
    page_head("Moderation update");
    echo "Couldn't moderate the post.<br>\n";
    echo mysql_error();
    page_tail();
}

?>
