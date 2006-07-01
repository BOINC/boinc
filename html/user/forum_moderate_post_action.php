<?php
/**
 * When a moderator does something to a post, this page actually
 * commits those changes to the database.
 **/

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");
require_once("../inc/forum_std.inc");

db_init();

$user = re_get_logged_in_user();

if (!$user->isSpecialUser(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page("You are not authorized to moderate this post.");
}

// See if "action" is provided - either through post or get
if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page("You must specify an action...");
    } else {
	$action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$post = new Post(get_int('id'));
$thread = $post->getThread();

if ($action=="hide"){
    $result = $post->hide();
    if ($thread->getPostCount() == 0) {
	$result = $thread->hide();
    }
} elseif ($action=="unhide"){
    $result = $post->unhide();
} elseif ($action=="move"){
    $destination_thread = new Thread(post_int('threadid'));
    $result = $post->move($destination_thread);
} else {
    error_page("Unknown action ");
}


if ($result) {
    if (post_str('reason', true)){
        send_moderation_email($post, post_str("reason"));
    }
    header('Location: forum_thread.php?id='.$thread->getID());
} else {
    error_page("Moderation failed");
}

?>
