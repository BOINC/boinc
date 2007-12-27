<?php

// stuff related to "buddy lists"

require_once("../inc/forum_db.inc");
require_once("../inc/profile.inc");

// tell src:
// You are about to add X as a friend.
// We will notify X, who wil have to confirm that you are friends
// (add message here)
// add/cancel
//
// send PM or email to dest saying
// subject: X added you as a friend on Project
// X added you as a friend on Project.

// X says: Y

// To confirm this friend request, please visit:
// Z
//
// Thanks -- Project
//
// To control the emails you receive from Project, please visit W
//
// link goes to page:
// You have a friend request.
// (show picture)
// Name (conutry)
// You and X have friends in common:
// buttons: Confirm, Ignore
// small links: Send a message, report this person

// no rate-limiting mechanism

function send_request() {
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
    $friend = BoincFriend::lookup($user->id, $destid);
    if ($friend) {
        error_page("Friend request already exists");
    }
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
        <input type=submit value=Cancel>
    ";
    page_tail();
}

// User really means it.  Make DB entry and send notification
//
function handle_add_confirm($user) {
    $destid = post_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page("No such user");

    $msg = post_str('message', true);
    if ($msg) $msg = strip_tags(process_user_text($msg));

    $now = time();
    $ret = BoincFriend::insert("(user_src, user_dest, message, create_time, reciprocated) values ($user->id, $destid, '$msg', $now, 0)");
    if (!$ret) {
        error_page("database error");
    }
    $ret = BoincNotify::insert("(userid, create_time, type, opaque) values ($destid, $now, ".NOTIFY_FRIEND_REQ.", $user->id)");
    if (!$ret) {
        error_page("Database error");
    }
    page_head("Friend request sent");
    echo "We have notified <b>$destuser->name</b> of your request.";
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
        If $srcuser->name is in fact your friend, please click Accept.
    ";
    $img_url = profile_user_thumb_url($srcuser);
    if ($img_url) {
        echo "<p><img src=$img_url><p>\n";
    }
    if (strlen($friend->message)) {
        echo "<p>$srcuser->name says: $friend->message<p>";
    }
    echo "
        <p>
        <a href=friend.php?action=accept&userid=$srcid>Accept</a> |
        <a href=friend.php?action=ignore&userid=$srcid>Ignore</a>
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
    $ret = BoincFriend::insert("(user_src, user_dest, message, create_time, reciprocated) values ($user->id, $srcid, '$msg', $now, 1)");
    if (!$ret) {
        error_page("database error");
    }
    $ret = BoincNotify::insert("(userid, create_time, type, opaque) values ($srcid, $now, ".NOTIFY_FRIEND_ACCEPT.", $user->id)");
    if (!$ret) {
        error_page("Database error");
    }

    $notify = BoincNotify::lookup($user->id, NOTIFY_FRIEND_REQ, $srcid);
    if ($notify) {
        $notify->delete();
    } else {
        echo "?? notification not found";
    }

    page_head("Friendship confirmed");
    echo "Your friendship with <b>$srcuser->name</b> has been confirmed.";
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
    } else {
        echo "?? notification not found";
    }
    page_head("Friendship declined");
    echo "You have declined friendship with <b>$srcuser->name</b>.";
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
default:
    error_page("unknown action");
}

?>
