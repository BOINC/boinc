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

if (!$_POST['action'] and $_GET['action']) $_POST['action']=$_GET['action'];

if (!$_POST['action']) {
    echo "You must specify an action...";
    exit();
} else {
    if (empty($_GET['id'])) {
        // TODO: Standard error page
        echo "Invalid post ID.<br>";
        exit();
    }
}

$post = getPost($_GET['id']);
$thread = getThread($post->thread);

if ($_POST['action']=="hide"){
    $result=mysql_query("update post set hidden = ".intval($_POST["category"])." where id=".$post->id);
    echo mysql_error();
} elseif ($_POST['action']=="unhide"){
    $result=mysql_query("update post set hidden = 0 where id=".$post->id);
    echo mysql_error();
} elseif ($_POST['action']=="move"){
    if (getThread($_POST['threadid'])){
        $result=mysql_query("update post set thread = ".intval($_POST['threadid'])." where id=".$post->id);
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
    if ($_POST['reason']){
        send_moderation_email(lookup_user_id($post->user),$thread, $post, $_POST["reason"]);
    }
    header('Location: forum_thread.php?id='.$thread->id);
} else {
    page_head("Moderation update");
    echo "Couldn't moderate the post.<br>\n";
    echo mysql_error();
    page_tail();
}

?>
