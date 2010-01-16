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

require_once("boinc_db.inc");
require_once("sanitize_html.inc");
require_once("bbcode_html.inc");

function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    echo "    | <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    echo "</div>\n";
}

function pm_create_new($error = null) {
    global $bbcode_html, $bbcode_js;
    global $g_logged_in_user;
    page_head(tra("Send private message"),'','','', $bbcode_js);
    
    if (post_str("preview", true) == tra("Preview")) {
        $options = new output_options;
        echo "<div id=\"preview\">\n";
        echo "<div class=\"header\">".tra("Preview")."</div>\n";
        echo output_transform(post_str("content", true), $options);
        echo "</div>\n";
    }
    
    $replyto = get_int("replyto", true);
    $userid = get_int("userid", true);
    
    $subject = null;
    $content = null;
    if ($replyto) {
        $message = BoincPrivateMessage::lookup_id($replyto);
        if (!$message || $message->userid != $g_logged_in_user->id) {
            error_page("no such message");
        }
        $content = "[quote]".$message->content."[/quote]\n";
        $userid = $message->senderid;
        $user = BoincUser::lookup_id($userid);
        if ($user != null) {
            $writeto = $userid." (".$user->name.")";
        }
        $subject = $message->subject;
        if (substr($subject, 0, 3) != "re:") {
            $subject = "re: ".$subject;
        }
    } elseif ($userid) {
        $user = BoincUser::lookup_id($userid);
        if ($user) {
            $writeto = $userid." (".$user->name.")";
        }
    } else {
        $writeto = post_str("to", true);
        $subject = post_str("subject", true);
        $content = post_str("content", true);
    }
    
    $content = htmlspecialchars($content);
    $subject = htmlspecialchars($subject);
    
    if ($error != null) {
        echo "<div class=\"error\">".$error."</div>\n";
    }
    
    echo "<form action=\"pm.php\" method=\"post\" name=\"post\" onsubmit=\"return checkForm(this)\">\n";
    echo "<input type=\"hidden\" name=\"action\" value=\"send\">\n";
    echo form_tokens($g_logged_in_user->authenticator);
    start_table();
    row2(tra("To")."<br /><span class=\"smalltext\">".tra("User IDs or unique usernames, separated with commas")."</span>",
        "<input type=\"text\" name=\"to\" value=\"$writeto\" size=\"60\">"
    );
    row2(tra("Subject"), "<input type=\"text\" name=\"subject\" value=\"$subject\" size=\"60\">");
    row2(tra("Message")."<span class=\"smalltext\">".html_info()."</span>",
        $bbcode_html."<textarea name=\"content\" rows=\"18\" cols=\"80\">$content</textarea>"
    );
    echo "<tr><td></td><td><input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\"> <input type=\"submit\" value=\"".tra("Send message")."\"></td></tr>\n";
    end_table();
    
    page_tail();
    exit();
}

function send_pm_notification_email(
    $logged_in_user, $to_user, $subject, $content
) {
    $message  = "
You have received a new private message at ".PROJECT.".

From: $logged_in_user->name (ID $logged_in_user->id)
Subject: $subject

$content

--------------------------
To delete or respond to this message, visit:
".URL_BASE."pm.php

To change email preferences, visit:
".URL_BASE."edit_forum_preferences_form.php
Do not reply to this message.
" ;
    send_email($to_user, "[".PROJECT."] private message", $message);
}

function pm_email_line($notify) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    return "$from_user->name sent you a private message; subject: '$pm->subject'";
}

function pm_web_line($notify) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    return "<a href=pm.php>Private message</a> from $from_user->name, subject: $pm->subject";
}

function pm_send($to_user, $subject, $content, $send_email) {
    global $g_logged_in_user;
    $sql_subject = boinc_real_escape_string(strip_tags($subject));
    $sql_content = boinc_real_escape_string($content);
    $mid = BoincPrivateMessage::insert("(userid, senderid, date, subject, content) VALUES ($to_user->id, $g_logged_in_user->id, UNIX_TIMESTAMP(), '$sql_subject', '$sql_content')");
    if (!$mid) {
        error_page("Couldn't create message");
    }
    // send email notification if needed
    //
    if ($send_email) {
        BoincForumPrefs::lookup($to_user);
        switch ($to_user->prefs->pm_notification) {
        case 0:
        case 2:
            break;
        case 1:
            send_pm_notification_email(
                $g_logged_in_user, $to_user, $subject, $content
            );
            break;
        }
    }

    // create notification in any case
    //
    BoincNotify::insert("(userid, create_time, type, opaque) values ($to_user->id, ".time().", ".NOTIFY_PM.", $mid)");
}

function pm_count($userid, $duration) {
    $time = time() - $duration;
    return BoincPrivateMessage::count("senderid=$userid AND date>$time");
}

function check_pm_count($userid) {
    if ((pm_count($userid, 60) >= 2) || (pm_count($userid, 600) >= 5) ||
        (pm_count($userid, 3600) >= 15) || (pm_count($userid, 86400) >= 50)) {
        error_page(tra("You are not allowed to send privates messages so often. Please wait some time before sending more messages."));
    }
}

function pm_notification($user) {
    $output = "";
    $unread = BoincPrivateMessage::count("userid=$user->id AND opened=0");
    
    $output .= "<a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>";
    if ($unread) {
        $output .= "<span class=\"inboxunread\"> ($unread ".tra("unread").")</span>\n";
    }
    $output .= " | <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    return $output;
}

function pm_email_remind($user) {
    if (!$user->prefs->pm_notification) {
        return '<br><span class="note">' .
            tra(
                "For email notification, %1edit community prefs%2",
                '<a href="edit_forum_preferences_form.php">', '</a>'
            ) .
            '</span>'
        ;
    }
    return "";
}

function pm_rss($notify, &$title, &$msg, &$url) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    $title = "Private message from $from_user->name: $pm->subject";
    $msg = "$pm->content";
    $url = URL_BASE."pm.php";
}

$cvs_version_tracker[]="\$Id: pm.inc 14019 2007-11-01 23:04:39Z davea $";
?>
