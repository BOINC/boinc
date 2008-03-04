<?php

// The following two are what gets put into notification email digests
//
function friend_notify_req_email_line($notify) {
    $src_user = BoincUser::lookup_id($notify->opaque);
    if (!$src_user) return "";
    return "$src_user->name has added you as a friend; please confirm";
}

function friend_notify_accept_email_line($notify) {
    $src_user = BoincUser::lookup_id($notify->opaque);
    if (!$src_user) return "";
    return "$src_user->name has confirmed you as a friend";
}

// The following two are what gets put in the Notification
// area of user's Account page
//
function friend_notify_req_web_line($notify) {
    $user = BoincUser::lookup_id($notify->opaque);
    return "
        <a href=friend.php?action=query&userid=$notify->opaque>Friendship request</a> from $user->name
    ";
}

function friend_notify_accept_web_line($notify) {
    $user = BoincUser::lookup_id($notify->opaque);
    return "
        <a href=friend.php?action=accepted&userid=$notify->opaque>Friendship confirmation</a> from $user->name
    ";
}

function send_friend_request_email($src_user, $dest_user, $msg) {
    $message  = "
$src_user->name has added you as a friend at ".PROJECT.".
";
    if (strlen($msg)) {
        $message .= "
$src_user->name says: $msg
";
    }

    $message .= "
Please accept or decline by visiting
".URL_BASE."home.php

--------------------------
To change email preferences, visit:
".URL_BASE."edit_forum_preferences_form.php
Do not reply to this message.
" ;
    send_email($dest_user, "[".PROJECT."] friend request", $message);
}

function send_friend_accept_email($dest_user, $src_user, $msg) {
    $message  = "
$dest_user->name has confirmed you as a friend at ".PROJECT.".
";
    if (strlen($msg)) {
        $message .= "
$dest_user->name says: $msg
";
    }

    $message .= "
Visit your Account page at
".URL_BASE."home.php

--------------------------
To change email preferences, visit:
".URL_BASE."edit_forum_preferences_form.php
Do not reply to this message.
" ;
    send_email($src_user, "[".PROJECT."] friend confirmed", $message);
}

function friend_req_rss($notify, &$title, &$msg, &$url) {
    $src_user = BoincUser::lookup_id($notify->opaque);
    $title = "Friend request";
    $msg = "$src_user->name has added you as a friend; please confirm or decline";
    $url = URL_BASE."friend.php?action=query&userid=$notify->opaque";
}

function friend_accept_rss($notify, &$title, &$msg, &$url) {
    $src_user = BoincUser::lookup_id($notify->opaque);
    $title = "Friendship confirmation";
    $msg = "$src_user->name has confirmed you as a friend";
    $url = URL_BASE."home.php";
}

?>
