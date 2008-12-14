<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
            error_page(tra("Already friends"));
        }
        $notify = BoincNotify::lookup($destuser->id, NOTIFY_FRIEND_REQ, $user->id);
        if ($notify) {
            page_head(tra("Request pending"));
            $t = date_str($friend->create_time);
            echo tra("You requested friendship with %1 on %2.", $destuser->name,$t) . "
                <p>" .
                tra("This request is still pending confirmation.");
            page_tail();
            exit();
        }
        BoincFriend::delete($user->id, $destuser->id);
    }
}

function check_ignoring($srcuser, $destuser) {
    BoincForumPrefs::lookup($destuser);
    if (is_ignoring($destuser, $srcuser)) {
        error_page(tra("%1 is not accepting friendship requests from you",$destuser->name));
    }
}

// user has clicked "add to friends".  Ask them if they really mean it.
//
function handle_add($user) {
    $destid = get_int('userid');
    if ($destid == $user->id) {
        error_page(tra("You can't be friends with yourself"));
    }
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page(tra("No such user"));

    check_pending($user, $destuser);
    check_ignoring($user, $destuser);

    page_head(tra("Add friend"));
    echo "
        <form method=post action=friend.php>
        <input type=hidden name=userid value=$destid>
        <input type=hidden name=action value=add_confirm>" .
        tra("You have asked to add %1 as a friend. We will notify %2 and will ask him/her to confirm that you are friends.",
	"<b>".$destuser->name."</b>","<b>".$destuser->name."</b>") ."
        <p>" .
        tra("Add an optional message here:") ."
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
    if (!$destuser) error_page(tra("No such user"));

    check_pending($user, $destuser);
    check_ignoring($user, $destuser);

    $msg = post_str('message', true);
    if ($msg) $msg = strip_tags(BoincDb::escape_string($msg));

    $now = time();
    $ret = BoincFriend::replace(
        "user_src=$user->id, user_dest=$destid, message='$msg', create_time=$now, reciprocated=0"
    );
    if (!$ret) {
        error_page(tra("Database error"));
    }
    $now = time();
    $type = NOTIFY_FRIEND_REQ;
    BoincNotify::replace("userid=$destid, create_time=$now, type=$type, opaque=$user->id");

    BoincForumPrefs::lookup($destuser);
    if ($destuser->prefs->pm_notification == 1) {
        send_friend_request_email($user, $destuser, $msg);
    }
    page_head(tra("Friend request sent"));
    echo tra("We have notified %1 of your request.","<b>".$destuser->name."</b>");
    page_tail();
}

// Show destination user the details of request, ask if they accept
//
function handle_query($user) {
    $srcid = get_int('userid');
    $srcuser = BoincUser::lookup_id($srcid);
    if (!$srcuser) error_page(tra("No such user"));
    $friend = BoincFriend::lookup($srcid, $user->id);
    if (!$friend) error_page(tra("Request not found"));
    page_head(tra("Friend request"));
    $x = user_links($srcuser, true);
    echo tra("%1 has added you as a friend.", $x);
    if (strlen($friend->message)) {
        echo "<p>".tra("%1 says: %2", $srcuser->name, $friend->message)."</p>";
    }
    echo "
        <p><ul class=\"actionlist\">";
    show_actionlist_button("friend.php?action=accept&userid=".$srcid, tra("Accept friendship"), tra("Click accept if %1 is in fact a friend", $srcuser->name));
    show_actionlist_button("friend.php?action=ignore&userid=".$srcid, tra("Decline"), tra("Click decline if %1 is not a friend", $srcuser->name));
    echo "    <p>
    ";
    page_tail();
}

// Here if they accepted
//
function handle_accept($user) {
    $srcid = get_int('userid');
    $srcuser = BoincUser::lookup_id($srcid);
    if (!$srcuser) error_page(tra("No such user"));

    $friend = BoincFriend::lookup($srcid, $user->id);
    if (!$friend) {
        error_page(tra("No request"));
    }
    $friend->update("reciprocated=1");

    // "accept message" not implemented in interface yet
    $msg = post_str('message', true);
    if ($msg) $msg = strip_tags(BoincDb::escape_string($msg));
    $now = time();
    $ret = BoincFriend::replace("user_src=$user->id, user_dest=$srcid, message='$msg', create_time=$now, reciprocated=1");
    if (!$ret) {
        error_page(tra("Database error"));
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

    page_head(tra("Friendship confirmed"));
    echo tra("Your friendship with %1 has been confirmed.","<b>" . $srcuser->name ."</b>");
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
    page_head(tra("Friendship declined"));
    echo tra("You have declined friendship with %1","<b>".$srcuser->name."</b>");
    page_tail();
}

// Here if initiator clicked on "confirmed" notification.
// Delete notification
//
function handle_accepted($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page(tra("No such user"));
    $notify = BoincNotify::lookup($user->id, NOTIFY_FRIEND_ACCEPT, $destid);
    if ($notify) {
        $notify->delete();
    } else {
        echo tra("Notification not found");
    }
    page_head(tra("Friend confirmed"));
    echo tra("You are now friends with %1.",$destuser->name);
    page_tail();
}

function handle_cancel_confirm($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page(tra("No such user"));
    page_head(tra("Cancel friendship?"));
    echo tra("Are you sure you want to cancel your friendship with %1?",$destuser->name) ."
        <p>
    <ul class=\"actionlist\">";
    show_actionlist_button("friend.php?action=cancel&userid=$destid", tra("Yes"), tra("Cancel friendship"));
    show_actionlist_button("home.php", tra("No"), tra("Stay friends"));
    echo "</ul>";
    page_tail();
}

function handle_cancel($user) {
    $destid = get_int('userid');
    $destuser = BoincUser::lookup_id($destid);
    if (!$destuser) error_page(tra("No such user"));
    BoincFriend::delete($user->id, $destid);
    page_head(tra("Friendship cancelled"));
    echo tra("Your friendship with %1 has been cancelled.",$destuser->name);
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
    error_page(tra("Unknown action"));
}

?>
