<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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
    show_image(REPORT_POST_IMAGE, tra("Block messages from this user"), tra("Block user"), REPORT_POST_IMAGE_HEIGHT);
    echo "</a>";
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

// show private messages,
// and delete notifications of new messages
//
function do_inbox($logged_in_user) {
    page_head(
        sprintf('%s: %s',
            tra("Private messages"),
            tra("Inbox")
        )
    );

    make_script();
    if (get_int("sent", true) == 1) {
        echo "<h3>".tra("Your message has been sent.")."</h3>\n";
    }
    $options = get_output_options($logged_in_user);

    BoincNotify::delete_aux("userid=$logged_in_user->id and type=".NOTIFY_PM);

    $msgs = BoincPrivateMessage::enum(
        "userid=$logged_in_user->id ORDER BY date DESC"
    );
    $nmsgs = count($msgs);
    if ($nmsgs == 0) {
        echo tra("You have no private messages.");
    } else {
        // see if we have to paginate messages
        //
        $nshow = $logged_in_user->prefs->display_wrap_postcount;
        if ($nshow < 1) $nshow = 20;
        $offset = 0;
        if ($nmsgs > $nshow) {
            $offset = get_int('offset', true);
            if ($offset === false) $offset = 0;
            if ($offset >= $nmsgs) $offset = 0;
            echo sprintf('Showing messages %d to %d of %d',
                $offset+1,
                min($offset+$nshow, $nmsgs),
                $nmsgs
            );
            if ($offset) {
                echo sprintf(
                    ' &middot; <a href=pm.php?action=inbox&offset=%d>Previous %d</a>',
                    max(0, $offset-$nshow), $nshow
                );
            }
            if ($offset+$nshow < $nmsgs) {
                echo sprintf(
                    ' &middot; <a href=pm.php?action=inbox&offset=%d>Next %d</a>',
                    $offset+$nshow, $nshow
                );
            }
        }

        echo "<form name=msg_list action=pm.php method=post>
            <input type=hidden name=action value=delete_selected>
        ";
        echo form_tokens($logged_in_user->authenticator);
        start_table('table-striped');
        row_heading_array(
            array(tra("Subject"), tra("Sender and date"), tra("Message")),
            array('style="width: 12em;"', 'style="width: 10em;"', "")
        );
        $i = 0;
        foreach($msgs as $msg) {
            if ($i<$offset) {
                $i++;
                continue;
            }
            if ($i>=$offset+$nshow) break;
            $i++;
            $sender = BoincUser::lookup_id($msg->senderid);
            if (!$sender) {
                $msg->delete();
                continue;
            }
            echo "<tr>\n";
            $checkbox = "<input type=checkbox name=pm_select_$msg->id>";
            if (!$msg->opened) {
                $msg->update("opened=1");
            }
            echo "<td valign=top> $checkbox $msg->subject </td>\n";
            echo "<td valign=top>".user_links($sender, BADGE_HEIGHT_SMALL);
            show_block_link($msg->senderid);
            echo "<br>".time_str($msg->date)."</td>\n";
            echo "<td valign=top>".output_transform($msg->content, $options)."<p>";
            $tokens = url_tokens($logged_in_user->authenticator);
            show_button("pm.php?action=new&amp;replyto=$msg->id", tra("Reply"), tra("Reply to this message"));
            show_button("pm.php?action=delete&amp;id=$msg->id&amp;$tokens", tra("Delete"), tra("Delete this message"));
            echo "</ul></td></tr>\n";
        }
        echo "
            <tr><td>
            <a href=\"javascript:set_all(1)\">".tra("Select all")."</a>
            |
            <a href=\"javascript:set_all(0)\">".tra("Unselect all")."</a>
            </td>
            <td colspan=2>
            <input class=\"btn btn-danger\" type=submit value=\"".tra("Delete selected messages")."\">
            </td></tr>
        ";
        end_table();
        echo "</form>\n";
    }
    page_tail();
}

// the following isn't currently used - we never show single messages
//
function do_read($logged_in_user) {
    $id = get_int("id");
    $message = BoincPrivateMessage::lookup_id($id);
    if (!$message || $message->userid != $logged_in_user->id) {
        error_page(tra("no such message"));
    }
    page_head(tra("Private messages")." : ".$message->subject);
    pm_header();

    $sender = BoincUser::lookup_id($message->senderid);

    start_table();
    echo "<tr><th>".tra("Subject")."</th><td>".$message->subject."</td></tr>";
    echo "<tr><th>".tra("Sender")."</th><td>".user_links($sender, BADGE_HEIGHT_SMALL);
    show_block_link($message->senderid);
    echo "</td></tr>";
    echo "<tr><th>".tra("Date")."</th><td>".time_str($message->date)."</td></tr>";
    echo "<tr><th>".tra("Message")."</th><td>".output_transform($message->content, $options)."</td></tr>";
    echo "<tr><td></td><td>\n";
    echo "<a href=\"pm.php?action=new&amp;replyto=$id\">".tra("Reply")."</a>\n";
    echo " &middot; <a href=\"pm.php?action=delete&amp;id=$id\">".tra("Delete")."</a>\n";
    echo " &middot; <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    end_table();

    if ($message->opened == 0) {
        $message->update("opened=1");
    }
    page_tail();
}

// form to send a personal message
//
function do_new($logged_in_user) {
    global $replyto, $userid;
    check_banished($logged_in_user);
    if (VALIDATE_EMAIL_TO_POST) {
        check_validated_email($logged_in_user);
    }
    pm_form_page($replyto, $userid);
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

function do_send_team($logged_in_user) {
    check_tokens($logged_in_user->authenticator);
    $subject = post_str("subject", true);
    $content = post_str("content", true);
    $teamid = post_int("teamid");
    if (post_str("preview", true) == tra("Preview")) {
        pm_team_form($logged_in_user, $teamid);
        return;
    }

    // make sure user is authorized, i.e. is a team admin
    //
    $team = BoincTeam::lookup_id($teamid);
    if (!$team) {
        error_page("no such team");
    }
    if (!is_team_admin($logged_in_user, $team)) {
        error_page("no team admin");
    }

    if (($subject == null) || ($content == null)) {
        pm_team_form(
            $logged_in_user, $teamid,
            tra("You need to fill all fields to send a private message")
        );
        return;
    }

    $subject = "Message from team ".$team->name.": ".$subject;
        // don't use tra() here because we don't know language of recipient
        // Also, we use it in pm_count() to exclude team messages from limit check
    $users = BoincUser::enum("teamid=$teamid");
    foreach ($users as $user) {
        pm_send_msg($logged_in_user, $user, $subject, $content, true);
    }
    page_head(tra("Message sent"));
    echo tra("Your message was sent to %1 team members.", count($users));
    page_tail();
}

function do_send($logged_in_user) {
    global $replyto, $userid;
    check_banished($logged_in_user);
    if (VALIDATE_EMAIL_TO_POST) {
        check_validated_email($logged_in_user);
    }
    check_tokens($logged_in_user->authenticator);

    $to = sanitize_tags(post_str("to", true));
    $subject = post_str("subject", true);
    $content = post_str("content", true);

    if (post_str("preview", true) == tra("Preview")) {
        pm_form_page($replyto, $userid);
        return;
    }
    if (($to == null) || ($subject == null) || ($content == null)) {
        pm_form_page(
            $replyto, $userid,
            tra("You need to fill all fields to send a private message")
        );
        return;
    }
    if (!akismet_check($logged_in_user, $content)) {
        pm_form_page($replyto, $userid,
            tra("Your message was flagged as spam by the Akismet anti-spam system.  Please modify your text and try again.")
        );
        return;
    }
    $usernames = explode("\n", $to);

    $userlist = array();
    $userids = array(); // To prevent from spamming a single user by adding it multiple times

    foreach ($usernames as $username) {
        // can be <id>, name, or '<id> (name)'
        // (PM reply fills in the latter)
        //
        $x = explode(' ', $username);
        if (is_numeric($x[0])) {     // user ID
            $userid = (int)$x[0];
            $user = BoincUser::lookup_id($userid);
            if ($user == null) {
                pm_form_page(
                    $replyto, $userid,
                    tra("Could not find user with id %1", $userid)
                );
                return;
            }
        } else {
            $users = BoincUser::lookup_name($username);
            if (count($users) == 0) {
                pm_form_page(
                    $replyto, $userid,
                    tra("Could not find user with username %1", $username)
                );
                return;
            } elseif (count($users) > 1) { // Non-unique username
                pm_form_page(
                    $replyto, $userid,
                    tra("%1 is not a unique username; you will have to use user ID", $username)
                );
                return;
            }
            $user = $users[0];
        }
        BoincForumPrefs::lookup($user);
        if (!is_moderator($logged_in_user) && is_ignoring($user, $logged_in_user)) {
            pm_form_page(
                $replyto, $userid,
                UNIQUE_USER_NAME
                ?tra("User %1 is not accepting private messages from you.",
                    $user->name
                )
                :tra("User %1 (ID: %2) is not accepting private messages from you.",
                    $user->name,
                    $user->id
                )
            );
            return;
        }
        if (!isset($userids[$user->id])) {
            $userlist[] = $user;
            $userids[$user->id] = true;
        }
    }

    foreach ($userlist as $user) {
        if (!is_moderator($logged_in_user, null)) {
            check_pm_count($logged_in_user->id);
        }
        pm_send_msg($logged_in_user, $user, $subject, $content, true);
    }

    Header("Location: pm.php?action=inbox&sent=1");
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
    echo "<input class=\"btn btn-default\" type=\"submit\" value=\"".tra("Add user to filter")."\">\n";
    echo "<a href=\"pm.php?action=inbox\">".tra("No, cancel")."</a>\n";
    echo "</form>\n";
    page_tail();
}

function do_confirmedblock($logged_in_user) {
    check_tokens($logged_in_user->authenticator);
    $id = post_int("id");
    $blocked_user = BoincUser::lookup_id($id);
    if (!$blocked_user) error_page(tra("no such user"));
    if (is_moderator($blocked_user)) {
        error_page(
            sprintf('%s is a moderator, and can\'t be blocked',
                $blocked_user->name
            )
        );
    }
    add_ignored_user($logged_in_user, $blocked_user);

    page_head(tra("User %1 blocked", $blocked_user->name));

    echo "<div>".tra("User %1 has been blocked from sending you private messages.", $blocked_user->name)."\n";
    echo tra("To unblock, visit %1 message board preferences %2", "<a href=\"edit_forum_preferences_form.php\">", "</a>")."</div>\n";
    page_tail();
}

function do_delete_selected($logged_in_user) {
    check_tokens($logged_in_user->authenticator);

    $msgs = BoincPrivateMessage::enum(
        "userid=$logged_in_user->id"
    );
    foreach($msgs as $msg) {
        $x = "pm_select_$msg->id";
        if (post_str($x, true)) {
            $msg = BoincPrivateMessage::lookup_id($msg->id);
            $msg->delete();
        }
    }
    Header("Location: pm.php?action=inbox&deleted=1");
}

$replyto = get_int("replyto", true);
$userid = get_int("userid", true);
$teamid = get_int("teamid", true);
if (!$teamid) {
    $teamid = post_int("teamid", true);
}

$action = sanitize_tags(get_str("action", true));
if (!$action) {
    $action = sanitize_tags(post_str("action", true));
}

if (!$action) {
    $action = "inbox";
}

if ($action == "inbox") {
    do_inbox($logged_in_user);
} elseif ($action == "read") {
    do_read($logged_in_user);
} elseif ($action == "new") {
    if (!$teamid) $teamid = post_int("teamid", true);
    if ($teamid) {
        pm_team_form($logged_in_user, $teamid);
    } else {
        do_new($logged_in_user);
    }
} elseif ($action == "delete") {
    do_delete($logged_in_user);
} elseif ($action == "send") {
    if ($teamid) {
        do_send_team($logged_in_user);
    } else {
        do_send($logged_in_user);
    }
} elseif ($action == "block") {
    do_block($logged_in_user);
} elseif ($action == "confirmedblock") {
    do_confirmedblock($logged_in_user);
} elseif ($action == "delete_selected") {
    do_delete_selected($logged_in_user);
} else {
    error_page(tra("Unknown action"));
}

?>
