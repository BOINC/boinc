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

require_once("../inc/boinc_db.inc");
require_once("../inc/email.inc");
require_once("../inc/pm.inc");
require_once("../inc/forum.inc");
require_once("../inc/akismet.inc");

function show_block_link($userid) {
    echo " <a href=\"pm.php?action=block&amp;id=$userid\">";
    show_image(REPORT_POST_IMAGE, "Block messages from this user",  "Block user", REPORT_POST_IMAGE_HEIGHT);
    echo "</a>";
}

$action = get_str("action", true);
if (!$action) {
    $action = post_str("action", true);
}

if (!$action) {
    $action = "inbox";
}

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

function make_script() {
    echo "
        <script type=\"text/javascript\">
        function set_all(val) {
            f = document.msg_list;
            n = f.elements.length;
            for (i=0; i<n; i++) {
                e = f.elements[i];
                if (e.type=='checkbox') {
                    e.checked = val;
                }
            }
        }
        </script>
    ";
}

// show all private messages,
// and delete notifications of new messages
//
function do_inbox($logged_in_user) {
    page_head(tra("Private messages").": ".tra("Inbox"));
    
    make_script();
    if (get_int("sent", true) == 1) {
        echo "<div class=\"notice\">".tra("Your message has been sent.")."</div>\n";
    }
    $options = get_output_options($logged_in_user);
    
    BoincNotify::delete_aux("userid=$logged_in_user->id and type=".NOTIFY_PM);

    $msgs = BoincPrivateMessage::enum(
        "userid=$logged_in_user->id ORDER BY date DESC"
    );
    if (count($msgs) == 0) {
        echo tra("You have no private messages.");
    } else {
        echo "<form name=msg_list action=pm.php method=post>
            <input type=hidden name=action value=delete_selected>
        ";
        echo form_tokens($logged_in_user->authenticator);
        start_table();
        echo "<tr><th>".tra("Subject")."</th><th>".tra("Sender and date")."</th><th>".tra("Message")."</th></tr>\n";
        $i = 0;
        foreach($msgs as $msg) {
            $i++;
            $class = ($i%2)? "row0": "row1";
            echo "<tr class=$class>\n";
            $checkbox = "<input type=\"checkbox\" name=\"pm_select[]\" value=\"".$msg->id."\">";
            if (!$msg->opened) {
                $msg->update("opened=1");
            }
            echo "<td valign=top> $checkbox $msg->subject </td>\n";
            echo "<td valign=top>".user_links(get_user_from_id($msg->senderid));
            show_block_link($msg->senderid);
            echo "<br>".time_str($msg->date)."</td>\n";
            echo "<td valign=top>".output_transform($msg->content, $options)."<p>";
            $tokens = url_tokens($logged_in_user->authenticator);
	    echo "<ul class=\"actionlist\">";
            show_actionlist_button("pm.php?action=new&amp;replyto=$msg->id", tra("Reply"), "Reply to this message");
            show_actionlist_button("pm.php?action=delete&amp;id=$msg->id&amp;$tokens", tra("Delete"), "Delete this message");
            echo "</ul></td></tr>\n";
        }
        echo "
            <tr><td>
            <a href=\"javascript:set_all(1)\">Select all</a>
            |
            <a href=\"javascript:set_all(0)\">Unselect all</a>
            </td>
            <td colspan=2>
            <input type=submit value=\"".tra("Delete selected messages")."\">
            </td></tr>
        ";
        end_table();
        echo "</form>\n";
    }
}

// the following isn't currently used - we never show single messages
//
function do_read($logged_in_user) {
    $id = get_int("id");
    $message = BoincPrivateMessage::lookup_id($id);
    if (!$message || $message->userid != $logged_in_user->id) {
        error_page("no such message");
    }
    page_head(tra("Private messages")." : ".$message->subject);
    pm_header();
    
    $sender = BoincUser::lookup_id($message->senderid);

    start_table();
    echo "<tr><th>".tra("Subject")."</th><td>".$message->subject."</td></tr>";
    echo "<tr><th>".tra("Sender")."</th><td>".user_links($sender);
    show_block_link($message->senderid);
    echo "</td></tr>";
    echo "<tr><th>".tra("Date")."</th><td>".time_str($message->date)."</td></tr>";
    echo "<tr><th>".tra("Message")."</th><td>".output_transform($message->content, $options)."</td></tr>";
    echo "<tr><td class=\"pm_footer\"></td><td>\n";
    echo "<a href=\"pm.php?action=new&amp;replyto=$id\">".tra("Reply")."</a>\n";
    echo " | <a href=\"pm.php?action=delete&amp;id=$id\">".tra("Delete")."</a>\n";
    echo " | <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    end_table();
    
    if ($message->opened == 0) {
        $message->update("opened=1");
    }
}

function do_new($logged_in_user) {
    check_banished($logged_in_user);
    pm_create_new();
}

function do_delete($logged_in_user) {
    $id = get_int("id", true);
    if ($id == null) {
        $id = post_int("id");
    }
    check_tokens($logged_in_user->authenticator);
    BoincPrivateMessage::delete_aux("userid=".$logged_in_user->id." AND id=$id");
    header("Location: pm.php");
}

function do_send($logged_in_user) {
    check_banished($logged_in_user);
    check_tokens($logged_in_user->authenticator);
    
    $to = post_str("to", true);
    $subject = post_str("subject", true);
    $content = post_str("content", true);
    
    if (post_str("preview", true) == tra("Preview")) {
        pm_create_new();
    }
    if (($to == null) || ($subject == null) || ($content == null)) {
        pm_create_new(tra("You need to fill all fields to send a private message"));
    } else {
        if (!akismet_check($logged_in_user, $content)) {
            pm_create_new("Your message was flagged as spam
                by the Akismet anti-spam system.
                Please modify your text and try again."
            );
        }
        $to = str_replace(", ", ",", $to); // Filter out spaces after separator
        $users = explode(",", $to);
        
        $userlist = array();
        $userids = array(); // To prevent from spamming a single user by adding it multiple times
        
        foreach ($users as $username) {
            $user = explode(" ", $username);
            if (is_numeric($user[0])) { // user ID is gived
                $userid = $user[0];
                $user = lookup_user_id($userid);
                if ($user == null) {
                    pm_create_new(tra("Could not find user with id %1", $userid));
                }
            } else {
                $user = lookup_user_name($username);
                if ($user == null) {
                    pm_create_new(tra("Could not find user with username %1", $username));
                } elseif ($user == -1) { // Non-unique username
                    pm_create_new(tra("%1 is not a unique username; you will have to use user ID", $username));
                }
            }
            BoincForumPrefs::lookup($user);
            if (is_ignoring($user, $logged_in_user)) {
                pm_create_new(tra("User %1 (ID: %2) is not accepting private messages from you.", $user->name, $user->id));
            }
            if ($userids[$user->id] == null) {
                $userlist[] = $user;
                $userids[$user->id] = true;
            }
        }
        
        foreach ($userlist as $user) {
            if (!is_moderator($logged_in_user, null)) {
                check_pm_count($logged_in_user->id);
            }
            pm_send($user, $subject, $content, true);
        }
        
        Header("Location: pm.php?action=inbox&sent=1");
    }
}

function do_block($logged_in_user) {
    $id = get_int("id");
    $user = BoincUser::lookup_id($id);
    if (!$user) {
        error_page(tra("No such user"));
    }
    page_head(tra("Really block %1?", $user->name));
    echo "<div>".tra("Are you really sure you want to block user %1 from sending you private messages?", $user->name)."<br>\n";
    echo tra("Please note that you can only block a limited amount of users.")."</div>\n";
    echo "<div>".tra("Once the user has been blocked you can unblock it using forum preferences page.")."</div>\n";
    
    echo "<form action=\"pm.php\" method=\"POST\">\n";
    echo form_tokens($logged_in_user->authenticator);
    echo "<input type=\"hidden\" name=\"action\" value=\"confirmedblock\">\n";
    echo "<input type=\"hidden\" name=\"id\" value=\"$id\">\n";
    echo "<input type=\"submit\" value=\"".tra("Add user to filter")."\">\n";
    echo "<a href=\"pm.php?action=inbox\">".tra("No, cancel")."</a>\n";
    echo "</form>\n";
}

function do_confirmedblock($logged_in_user) {
    check_tokens($logged_in_user->authenticator);
    $id = post_int("id");
    $blocked_user = BoincUser::lookup_id($id);
    if (!$blocked_user) error_page("no such user");
    add_ignored_user($logged_in_user, $blocked_user);
    
    page_head(tra("User %1 blocked", $blocked_user->name));
    
    echo "<div>".tra("User %1 has been blocked from sending you private messages.", $blocked_user->name)."\n";
    echo tra("To unblock, visit %1message board preferences%2", "<a href=\"edit_forum_preferences_form.php\">", "</a>")."</div>\n";
}

function do_delete_selected($logged_in_user) {
    check_tokens($logged_in_user->authenticator);
    foreach ($_POST["pm_select"] as $id) {
        $id = BoincDb::escape_string($id);
        $msg = BoincPrivateMessage::lookup_id($id);
        if ($msg && $msg->userid == $logged_in_user->id) {
            $msg->delete();
        }
    }
    Header("Location: pm.php?action=inbox&deleted=1");
}

if ($action == "inbox") {
    do_inbox($logged_in_user);
} elseif ($action == "read") {
    do_read($logged_in_user);
} elseif ($action == "new") {
    do_new($logged_in_user);
} elseif ($action == "delete") {
    do_delete($logged_in_user);
} elseif ($action == "send") {
    do_send($logged_in_user);
} elseif ($action == "block") {
    do_block($logged_in_user);
} elseif ($action == "confirmedblock") {
    do_confirmedblock($logged_in_user);
} elseif ($action == "delete_selected") {
    do_delete_selected($logged_in_user);
} else {
    error_page("Unknown action");
}

page_tail();

$cvs_version_tracker[]="\$Id: pm.php 14077 2007-11-03 04:26:47Z davea $";
?>
