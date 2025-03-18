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

define('PM_LH_PCT', '30%');

function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    echo "    &middot; <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    echo "</div>\n";
}

function pm_rules() {
    if (defined('PM_RULES')) return PM_RULES;
    $x = '<span style="text-align: left"><p>';
    $x .= tra("
        <ul>
        <li> Messages may not contain content that is obscene, hate-related,
            sexually explicit or suggestive.
        <li> No commercial advertisements.
        <li> No links to web sites involving sexual content,
            gambling, or intolerance of others.
        <li> No messages intended to annoy or antagonize other people.
        <li> No messages that are deliberately hostile, threatening, or insulting.
        <li> No abusive comments involving race, religion,
            nationality, gender, class or sexuality.
        <li> The privileges of violators may be suspended or revoked.
        <li> If your account is suspended, don't create a new one.
        </ul>
    ");
    $x .= "</span>";
    return $x;
}

function pm_team_form($user, $teamid, $error=null) {
    global $bbcode_html, $bbcode_js;
    $team = BoincTeam::lookup_id($teamid);
    if (!$team) {
        error_page("no such team");
    }
    if (!is_team_admin($user, $team)) {
        error_page("not admin");
    }

    page_head(tra("Send message to team"),'','','', $bbcode_js);

    $subject = post_str("subject", true);
    $content = post_str("content", true);
    if (post_str("preview", true) == tra("Preview")) {
        panel(tra('Preview'),
            function() use($content) {
                echo output_transform($content, null);
            }
        );
    }
    if ($error) {
        echo "<p class=\"text-danger\">".$error."</p>\n";
    }

    echo "<form action=\"pm.php\" method=\"post\" name=\"post\" onsubmit=\"return checkForm(this)\">\n";
    echo "<input type=\"hidden\" name=\"action\" value=\"send\">\n";
    echo "<input type=\"hidden\" name=\"teamid\" value=\"$teamid\">\n";
    echo form_tokens($user->authenticator);
    start_table();
    row2(
        tra("Subject"),
        "<input type=\"text\" class=\"form-control\" name=\"subject\" value=\"$subject\">",
        null, PM_LH_PCT
    );
    row2(
        tra("Message")."<small>".bbcode_info()."</small>",
        $bbcode_html."<textarea name=\"content\" class=\"form-control\" rows=\"18\">$content</textarea>",
        null, PM_LH_PCT
    );
    row2(
        '',
        sprintf(
            '<input class="btn" %s type="submit" name="preview" value="%s">
            <input class="btn" %s type="submit" value="%s">
            ',
            button_style('blue'),
            tra("Preview"),
            button_style(),
            tra("Send message")
        ),
        null, PM_LH_PCT
    );
    end_table();
    page_tail();
}

// show the send-PM page, possibly with an error message
//
function pm_form_page($replyto, $userid, $error = null) {
    global $bbcode_html, $bbcode_js;
    global $g_logged_in_user;
    page_head(tra("Send private message"),'','','', $bbcode_js);

    if (post_str("preview", true) == tra("Preview")) {
        $content = post_str("content", true);
        panel(tra('Preview'),
            function() use($content) {
                echo output_transform($content, null);
            }
        );
    }

    $subject = '';
    $content = '';
    if ($replyto) {
        $message = BoincPrivateMessage::lookup_id($replyto);
        if (!$message || $message->userid != $g_logged_in_user->id) {
            error_page(tra("no such message"));
        }
        $content = "[quote]".$message->content."[/quote]\n";
        $userid = $message->senderid;
        $user = BoincUser::lookup_id($userid);
        if (!$user) {
            error_page("Sender no longer exists");
        }
        $writeto = UNIQUE_USER_NAME?$user->name:$userid." (".$user->name.")";
        $subject = $message->subject;
        if (substr($subject, 0, 3) != "re:") {
            $subject = "re: ".$subject;
        }
    } elseif ($userid) {
        $user = BoincUser::lookup_id($userid);
        if (!$user) {
            error_page("Sender no longer exists");
        }
        if (is_banished($user)) {
            echo sprintf(
                "<p>
                This user is banished until %s.
                You can send them a message, but they can't reply until then.
                <p>",
                time_str($user->prefs->banished_until)
            );
        }
        $writeto = UNIQUE_USER_NAME?$user->name:$userid." (".$user->name.")";
    } else {
        $writeto = sanitize_tags(post_str("to", true));
        $subject = post_str("subject", true);
        $content = post_str("content", true);
    }

    $content = $content?htmlspecialchars($content):'';
    $subject = $subject?htmlspecialchars($subject):'';

    if ($error != null) {
        echo "<p class=\"text-danger\">".$error."</p>\n";
    }

    echo "<form action=\"pm.php\" method=\"post\" name=\"post\" onsubmit=\"return checkForm(this)\">\n";
    echo "<input type=\"hidden\" name=\"action\" value=\"send\">\n";
    echo form_tokens($g_logged_in_user->authenticator);
    start_table();
    row2(
        sprintf('%s <br><small>%s</small>',
            tra("To"),
            UNIQUE_USER_NAME
                ?tra('User names, one per line')
                :tra("User IDs or unique usernames, one per line")
        ),
        sprintf(
            '<textarea rows=2 class="form-control" name="to">%s</textarea>',
            $writeto
        ),
        null, PM_LH_PCT
    );
    row2(
        tra("Subject"),
        "<input type=\"text\" class=\"form-control\" name=\"subject\" value=\"$subject\">",
        null, PM_LH_PCT
    );
    row2(
        tra("Message")."<small>".bbcode_info().pm_rules()."</small>",
        sprintf(
            '%s <textarea name="content" class="form-control" rows="18">%s</textarea>',
            $bbcode_html,
            $content
        ),
        null, PM_LH_PCT
    );
    row2(
        '',
        sprintf(
            '<input class="btn btn-primary" type="submit" name="preview" value="%s">
                <input class="btn btn-success" type="submit" value="%s">
            ',
            tra("Preview"),
            tra("Send message")
        ),
        null, PM_LH_PCT
    );
    end_table();

    page_tail();
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
".secure_url_base()."pm.php

To change email preferences, visit:
".secure_url_base()."edit_forum_preferences_form.php
Do not reply to this message.
" ;
    send_email($to_user, "[".PROJECT."] - private message", $message);
}

function pm_email_line($notify) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    if (!$pm || !$from_user) return null;
    return "$from_user->name ".tra("sent you a private message; subject:")." '$pm->subject'";
}

function pm_web_line($notify) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    if (!$pm || !$from_user) return null;
    return "<a href=pm.php>".tra("Private message%1 from %2, subject:" , "</a>", $from_user->name )." $pm->subject";
}

function pm_send_msg($from_user, $to_user, $subject, $content, $send_email) {
    $sql_subject = BoincDb::escape_string(sanitize_tags($subject));
    $sql_content = BoincDb::escape_string($content);
    $mid = BoincPrivateMessage::insert("(userid, senderid, date, subject, content) VALUES ($to_user->id, $from_user->id, UNIX_TIMESTAMP(), '$sql_subject', '$sql_content')");
    if (!$mid) {
        error_page(tra("Couldn't create message"));
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
                $from_user, $to_user, $subject, $content
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

    // we don't want to include team messages in this count.
    // Kludge for excluding them based on subject.
    // Should add a flag to private_message to distinguish them.
    //
    return BoincPrivateMessage::count(
        "senderid=$userid AND date>$time AND subject not like 'Message from team%'"
    );
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
    $output .= " &middot; <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    return $output;
}

function pm_email_remind($user) {
    if (!$user->prefs->pm_notification) {
        return "<br><small>" .
            tra(
                "For email notification, %1 edit community prefs %2",
                '<a href="edit_forum_preferences_form.php">', '</a>'
            ) .
            "</small>"
        ;
    }
    return "";
}

function pm_rss($notify, &$title, &$msg, &$url) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    if (!$pm || !$from_user) {
        $msg = null;
        return;
    }
    $title = tra("Private message");
    $msg = "You have received a <a href=".secure_url_base()."pm.php>private message</a>.";
    $url = secure_url_base()."pm.php";
}

function pm_delete_user($user) {
    $mm = BoincPrivateMessage::enum("userid=$user->id or senderid=$user->id");
    foreach ($mm as $m) {
        $m->delete();
    }
}

?>
