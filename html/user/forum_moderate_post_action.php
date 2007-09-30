<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

/**
 * When a moderator does something to a post, this page actually
 * commits those changes to the database.
 **/

require_once("../inc/forum.inc");
require_once("../inc/forum_email.inc");
require_once("../inc/forum_std.inc");

db_init();

$user = re_get_logged_in_user();
check_tokens($user->getAuthenticator());

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
    if ($thread->getPostCount() == 0) {
	$thread->hide();
    }
} elseif ($action=="banish_user"){
    if (!$user->isSpecialUser(S_ADMIN)) {
      // Can't banish without being administrator
      error_page("You are not authorized to banish this user.");
    }
    $userid = post_int('userid');
    $user = newUser($userid);
    if (!$user) {
        error_page("no user");
    }
    $duration = post_int('duration');
    if ($duration == -1) {
        $t = 2147483647; // Maximum integer value
    } else {
        $t = time() + $duration;
    }
    $reason = post_str("reason", true);
    $query = "update forum_preferences set banished_until=$t where userid=$userid";
    $result = mysql_query($query);
    if ($result) {
        echo "User $user->name has been banished.";
        send_banish_email($user, $t, $reason);
    } else {
        echo "DB failure for $query";
        echo mysql_error();
    }
    page_tail();
    exit();
} else {
    error_page("Unknown action ");
}

switch (post_int("category", true)) {
    case 1:
        $mod_category = "Obscene";
    case 2:
        $mod_category = "Flame/Hate mail";
    case 3:
        $mod_category = "Commercial spam";
    case 4:
        $mod_category = "Doublepost";
    case 5:
        $mod_category = "User Request";
    default:
        $mod_category = "Other";
}

if ($result) {
    if (post_str('reason', true)){
        send_moderation_email($post, "Category: ".$mod_category."\n".post_str("reason"), $action);
    } else { 
        send_moderation_email($post, "Category: ".$mod_category."\n"."None given", $action);
    }
    header('Location: forum_thread.php?id='.$thread->getID());
} else {
    error_page("Moderation failed");
}

?>
