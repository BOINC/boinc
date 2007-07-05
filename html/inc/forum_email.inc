<?php

require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../project/project.inc");

// Determine if phpmailer is installed and to be used (defined in project.inc)
if (isset($USE_PHPMAILER)) {
    if ($USE_PHPMAILER) {
        require_once("../inc/phpmailer/class.phpmailer.php");
    }
} else {
    $USE_PHPMAILER = false;
}

if (FORUM_MODERATION_EMAIL) {
}

/**
 * Send mail using the new User object style
 **/
function re_send_email($user, $subject, $body) {
    global $USE_PHPMAILER;
    global $PHPMAILER_HOST;
    global $PHPMAILER_MAILER;

    if ($USE_PHPMAILER) {
        $mail = new PHPMailer();
        $mail->AddAddress($user->getEmail(), $user->getName());
        $mail->Subject = $subject;
        $mail->Body = $body;
        $mail->From = EMAIL_FROM;
        $mail->FromName = EMAIL_FROM;
        $mail->Host = $PHPMAILER_HOST;
        $mail->Mailer = $PHPMAILER_MAILER;
        return $mail->Send();
    } else {
        if (defined('EMAIL_FROM')) {
            $headers = "From: ". EMAIL_FROM;
        } else {
            $headers ="";
        }
        return mail($user->getEmail(), $subject, $body, $headers);
    }
}

/**
 * If a post is hidden, this function emails the author
 * to let them know what happened.
 **/
function send_moderation_email($post, $message, $action) {
    $moderator=re_get_logged_in_user();
    $body = "";
    $thread = $post->getThread();
    $user = $post->getOwner();
    $subject = PROJECT." forum ".$action." notice";
    $body = PROJECT." notification:
    $ownerid = $user->getID();

This email is to inform you that one of your posts in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->getTitle()." 
    Post:           ".$post->getID()." by ".$user->getID()." (".$user->getName().")
    Link:           ".URL_BASE."/forum_thread.php?id=".$thread->getID()."
    Moderator:      ".$moderator->getName()." (".$moderator->getID().")
    Action:         ".$action."

The moderator gave this explanation to why your post was moderated:
".$message;
$body .= "

This was the content of your post:
".$post->getContent()."

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    $emails = explode("|", POST_REPORT_EMAILS);
    $success = true;
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, "POST ".$action." REPORT: ".$thread->getTitle(), $body)) {
            $success = false;
        }
    }
    return re_send_email($user, $subject, $body);
}

/**
 * If an entire thread is hidden, this function emails the owner
 * to let them know what happened.
 */
function send_thread_moderation_email($thread, $message, $action) {
    $moderator=re_get_logged_in_user();
    $user = $thread->getOwner();
    $body = "";

    $subject = PROJECT." forum moderation notice";
    $body = PROJECT." notification:

This email is sent to inform you that one of your threads in the forum has been affected by moderation in ".PROJECT.":
    Thread:         ".$thread->getTitle()."
    Link:           ".MASTER_URL."forum_thread.php?id=".$thread->getID()."
    Moderator:      ".$moderator->getName()." (".$moderator->getID().")
    Action:         ".$action."

The moderator gave this explanation to why your thread was moderated:
".$message;
$body .= "

For further information and assistance with ".PROJECT." go to ".MASTER_URL;
    $emails = explode("|", POST_REPORT_EMAILS);
    foreach ($emails as $email) {
        $admin->email_addr = $email;
        if (!send_email($admin, "THREAD ".$action." REPORT: ".$thread->getTitle(), $body)) {
            $success = false;
        }
    }
    return re_send_email($user, $subject, $body);
}

/**
 * If a user is subscribed to a thread that is replied to, this
 * function sends them an email notifying them of the reply.
 **/
function send_reply_notification_email($thread, $user){
    $title = PROJECT . ": A user has posted to '". stripslashes($thread->getTitle()) ."'";
    $link = URL_BASE . "/forum_thread.php?id=" . $thread->getID();
    $body = "Another " . PROJECT . " user has posted to the thread \"" . stripslashes($thread->getTitle()) . "\".\n"
           ."To view the updated thread, visit the following URL:\n\n$link";
    return re_send_email($user, $title, $body);
}

/**
 * When a user clicks the red "x" to notify moderators about a post,
 * this function generates the email to let the moderators know about
 * it.
 **/
function send_report_post_email($user, $thread,  $post, $message) {
    $body = "";
    $owner = $post->getOwner();

    $subject = PROJECT." post in '".$thread->getTitle()."' reported as offensive";
    $body = PROJECT." notification:

This email is sent to inform you that one of the posts in the forum was reported as offensive in ".PROJECT.":
    Thread:         ".$thread->getTitle()."
    Post:           ".$post->getID()." by ".$owner->getID()." (".$owner->getName().")
    Reporting User: ".$user->getID()." (".$user->getName().")
    Link:           ".URL_BASE."/forum_thread.php?id=".$thread->getID()."#".$post->getID()."
    
The reporting user gave this explanation to why the post was reported:
".$message."

This was the contents of the post:
".$post->getContent()."

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
        if (!send_email($admin, $user->getName()." has been banished.", $body)) {
            $success = false;
        }
    }
    return re_send_email($user, $subject, $body);
}

?>
