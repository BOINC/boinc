<?php

require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../project/project.inc");

define('MASTER_URL', $master_url);

// If a post is hidden, this function emails the author
// to let them know what happened.
//
function send_moderation_email($post, $thread, $message, $action) {
    $moderator=get_logged_in_user();
    $body = "";
    $user = BoincUser::lookup_id($post->user);
    $subject = PROJECT." forum ".$action." notice";
    $body = PROJECT." notification:
    $ownerid = $user->id;

This email is to inform you that one of your posts in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title." 
    Post:           ".$post->id." by ".$user->id." (".$user->name.")
    Link:           ".URL_BASE."/forum_thread.php?id=".$thread->id."
    Moderator:      ".$moderator->name." (".$moderator->id.")
    Action:         ".$action."

The moderator gave this explanation to why your post was moderated:
".$message;
$body .= "

This was the content of your post:
".$post->content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    $emails = explode("|", POST_REPORT_EMAILS);
    $success = true;
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, "POST ".$action." REPORT: ".$thread->title, $body)) {
            $success = false;
        }
    }
    return send_email($user, $subject, $body);
}

// If an entire thread is hidden, this function emails the owner
// to let them know what happened.
//
function send_thread_moderation_email($thread, $message, $action) {
    $moderator=get_logged_in_user();
    $user = BoincUser::lookup_id($thread->owner);
    $body = "";

    $subject = PROJECT." forum moderation notice";
    $body = PROJECT." notification:

This email is sent to inform you that one of your threads in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->title."
    Link:           ".MASTER_URL."forum_thread.php?id=".$thread->id."
    Moderator:      ".$moderator->name." (".$moderator->id.")
    Action:         ".$action."

The moderator gave this explanation to why your thread was moderated:
".$message;
$body .= "

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    $emails = explode("|", POST_REPORT_EMAILS);
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, "THREAD ".$action." REPORT: ".$thread->title, $body)) {
            $success = false;
        }
    }
    return send_email($user, $subject, $body);
}

// If a user is subscribed to a thread that is replied to, this
// function sends them an email notifying them of the reply.
//
function send_reply_notification_email($thread, $user){
    $title = PROJECT . ": A user has posted to '". stripslashes($thread->title) ."'";
    $link = URL_BASE . "/forum_thread.php?id=" . $thread->id;
    $body = "Another " . PROJECT . " user has posted to the thread \"" . stripslashes($thread->title) . "\".\n"
           ."To view the updated thread, visit the following URL:\n\n$link";
    return send_email($user, $title, $body);
}

// When a user clicks the red "x" to notify moderators about a post,
// this function generates the email to let the moderators know about it.
//
function send_report_post_email($user, $thread,  $post, $message) {
    $body = "";
    $owner = BoincUser::lookup_id($post->user);

    $subject = PROJECT." post in '".$thread->title."' reported as offensive";
    $body = PROJECT." notification:

This email is sent to inform you that one of the posts in the forum was reported as offensive in ".PROJECT.":
    Thread:         ".$thread->title."
    Post:           ".$post->id." by ".$owner->id." (".$owner->name.")
    Reporting User: ".$user->id." (".$user->name.")
    Link:           ".URL_BASE."/forum_thread.php?id=".$thread->id."#".$post->id."
    
The reporting user gave this explanation to why the post was reported:
".$message."

This was the contents of the post:
".$post->content."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;

    //Security check, do we have an admin?
    if (!defined("POST_REPORT_EMAILS")) error_page("This project has not yet defined an administrator to handle this kind of forum reports. Please contact the project and tell them to add this information in their html/project/project.inc file");
    $emails = explode("|", POST_REPORT_EMAILS);
    $success = true;
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, $subject, $body)) {
            $success = false;
        }
    }
    return success;
}

function send_banish_email($user, $duration, $reason) {
    $subject = PROJECT." posting privileges suspended";
    $body = "
This email is to inform you that you will not be able to
post to the ".PROJECT." message boards until ".date('M j, Y G:i', $duration).",
because your postings have not followed our guidelines.
    ";
    if ($reason) {
        $body .= "\n\nThe moderator gave the following explanation about your suspension:\n";
        $body .= $reason;
    }
    $emails = explode("|", POST_REPORT_EMAILS);
    $success = true;
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, $user->name." has been banished.", $body)) {
            $success = false;
        }
    }
    return send_email($user, $subject, $body);
}

function send_banish_vote_email($user, $duration, $reason, $end_time) {
    $now=time();
    $subject = PROJECT." banishment vote underway";
    $vote_url=parse_config(get_config(), "<master_url>")."forum_banishment_vote.php";
    $body = "
This email if to inform you that a vote has been started
regarding your banishment from the ".PROJECT." message boards until ".date('M j,
Y G:i', $duration+$now).",
because your postings have not followed our guidelines.

This vote will be underway until ".date('M j, Y G:i',$end_time)." or until a majority
decision has been reached.  If the vote does not result in banishment, you will be
able to resume posting at that time.
    ";
    if ($reason) {
        $body .= "\n\nThe moderator gave the following reason for your pending suspension:\n";
        $body .= $reason;
    }
    $success = send_email($user, $subject, $body);
    $emails = explode("|", POST_REPORT_EMAILS);
    $success = true;
    $body .= "\n\n<a href=".$vote_url."?action=yes&userid="
             .$user->id
           .">[vote to banish author]</a>\n\n"
           ."<a href=".$vote_url."?action=no&userid="
             .$user->id
           .">[vote not to banish author]</a>";

    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, "A banishment vote for ".$user->name." has been started.", $body)) {
            $success = false;
        }
    }
}
 


?>
