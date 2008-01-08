<?php

// stuff related to "buddy lists"

require_once("../inc/forum_db.inc");
require_once("../inc/profile.inc");

// see if there's already a request,
// and whether the notification record is there
//
function check_pending($user, $destuser) {
    $friend = BoincFriend::lookup($user->id, $destuser->id);
    if ($friend) {
        if ($friend->reciprocated) {
            error_page("Already friends");
        }
        $notify = BoincNotify::lookup($destuser->id, NOTIFY_FRIEND_REQ, $user->id);
        if ($notify) {
            page_head("Request pending");
            $t = date_str($friend->create_time);
            echo "You requested friendship with $destuser->name on $t.
                <p>
                This request is still pending confirmation.
            ";
            page_tail();
            exit();
        }
        BoincFriend::delete($user->id, $destuser->id);
    }
}

function check_ignoring($srcuser, $destuser) {
    BoincForumPrefs::lookup($destuser);
    if (is_ignoring($destuser, $srcuser)) {
        error_page("$destuser->name is not accepting friendship requests from you");
    }
}

// user has clicked "add to friends".  Ask them if they really mean it.
//
function handle_add($user) {
    $destid = get_int('userid');
    if ($destid == $user->id) {
        error_page("You can't be friends with yourself");
    }
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");

    check_pending($user, $destuser);
    check_ignoring($user, $destuser);

    page_head("Add friend");
    echo "
        <form method=post action=friend.php>
        <input type=hidden name=userid value=$destid>
        <input type=hidden name=action value=add_confirm>
        You have asked to add <b>$destuser->name</b> as a friend.
        We will notify <b>$destuser->name</b> and will ask him/her to
        confirm that you are friends.
        <p>
        Add an optional message here:
        <br>
        <textarea name=message cols=64 rows=4></textarea>
        <p>
        <input type=submit value=OK>
        </form>
    ";
    page_tail();
}

// User really means it.  Make DB entry and send notification
//
function handle_add_confirm($user) {
    $destid = post_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");

    check_pending($user, $destuser);
    check_ignoring($user, $destuser);

    $msg = post_str('message', true);
    if ($msg) $msg = strip_tags(process_user_text($msg));

    $now = time();
    $ret = BoincFriend::replace("user_src=$user->id, user_dest=$destid, message='$msg', create_time=$now, reciprocated=0");
    if (!$ret) {
        error_page("database error");
    }
    $now = time();
    $type = NOTIFY_FRIEND_REQ;
    BoincNotify::replace("userid=$destid, create_time=$now, type=$type, opaque=$user->id");

    BoincForumPrefs::lookup($destuser);
    if ($destuser->prefs->pm_notification == 1) {
        send_friend_request_email($user, $destuser, $msg);
    }
    page_head("Friend request sent");
    echo "
        We have notified <b>$destuser->name</b> of your request.
    ";
    page_tail();
}

// Show destination user the details of request, ask if they accept
//
function handle_query($user) {
    $srcid = get_int('userid');
    $srcuser = BoincUser::lookup_id($srcid);
    if (!$srcuser) error_page("No such user");
    $friend = BoincFriend::lookup($srcid, $user->id);
    if (!$friend) error_page("Request not found");
    page_head("Friend request");
    $x = user_links($srcuser, true);
    echo "
        $x has added you as a friend.
    ";
    if (strlen($friend->message)) {
        echo "<p>$srcuser->name says: $friend->message<p>";
    }
    echo "
        <p>
        <a href=friend.php?action=accept&userid=$srcid>Accept</a>
        (click if $srcuser->name is in fact a friend)
        <p>
        <a href=friend.php?action=ignore&userid=$srcid>Decline</a>
        (click if $srcuser->name is not a friend)
        <p>
    ";
    page_tail();
}

// Here if they accepted
//
function handle_accept($user) {
    $srcid = get_int('userid');
    $srcuser = BoincUser::lookup_id($srcid);
    if (!$srcuser) error_page("No such user");

    $friend = BoincFriend::lookup($srcid, $user->id);
    if (!$friend) {
        error_page("No request");
    }
    $friend->update("reciprocated=1");

    // "accept message" not implemented in interface yet
    $msg = post_str('message', true);
    if ($msg) $msg = strip_tags(process_user_text($msg));
    $now = time();
    $ret = BoincFriend::replace("user_src=$user->id, user_dest=$srcid, message='$msg', create_time=$now, reciprocated=1");
    if (!$ret) {
        error_page("database error");
    }
    $type = NOTIFY_FRIEND_ACCEPT;
    BoincNotify::replace("userid=$srcid, create_time=$now, type=$type, opaque=$user->id");
    BoincForumPrefs::lookup($srcuser);
    if ($srcuser->prefs->pm_notification == 1) {
        send_friend_accept_email($user, $srcuser, $msg);
    }

    $notify = BoincNotify::lookup($user->id, NOTIFY_FRIEND_REQ, $srcid);
    if ($notify) {
        $notify->delete();
    }

    page_head("Friendship confirmed");
    echo "
        Your friendship with <b>$srcuser->name</b> has been confirmed.
    ";
    page_tail();
}

// Here if they declined
//
function handle_ignore($user) {
    $srcid = get_int('userid');
    $srcuser = BoincUser::lookup_id($srcid);
    if (!$srcuser) error_page("No such user");
    $friend = BoincFriend::lookup($srcid, $user->id);
    if (!$friend) {
        error_page("No request");
    }
    $notify = BoincNotify::lookup($user->id, NOTIFY_FRIEND_REQ, $srcid);
    if ($notify) {
        $notify->delete();
    }
    page_head("Friendship declined");
    echo "
        You have declined friendship with <b>$srcuser->name</b>
    ";
    page_tail();
}

// Here if initiator clicked on "confirmed" notification.
// Delete notification
//
function handle_accepted($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");
    $notify = BoincNotify::lookup($user->id, NOTIFY_FRIEND_ACCEPT, $destid);
    if ($notify) {
        $notify->delete();
    } else {
        echo "?? notification not found";
    }
    page_head("Friend confirmed");
    echo "
        You are now friends with $destuser->name.
    ";
    page_tail();
}

function handle_cancel_confirm($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");
    page_head("Cancel friendship?");
    echo "
        Are you sure you want to cancel your friendship with $destuser->name?
        <p>
    ";
    show_button("friend.php?action=cancel&userid=$destid", "Yes", "Cancel friendship");
    echo "<p>";
    show_button("home.php", "No", "Don't cancel friendship");
    page_tail();
}

function handle_cancel($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");
    BoincFriend::delete($user->id, $destid);
    page_head("Friendship cancelled");
    echo "
        Your friendship with $destuser->name has been cancelled.
    ";
    page_tail();
}

// "home page" has Requests area
// (icon) N friend request(s)

$user = get_logged_in_user();

$action = get_str('action', true);
if (!$action) $action = post_str('action');

switch ($action) {
case 'add':
    handle_add($user);
    break;
case 'add_confirm':
    handle_add_confirm($user);
    break;
case 'query':
    handle_query($user);
    break;
case 'accept':
    handle_accept($user);
    break;
case 'accepted':
    handle_accepted($user);
    break;
case 'ignore':
    handle_ignore($user);
    break;
case 'cancel_confirm':
    handle_cancel_confirm($user);
    break;
case 'cancel':
    handle_cancel($user);
    break;
default:
    error_page("unknown action");
}

?>
