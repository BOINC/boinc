<?php

require_once("boinc_db.inc");

function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    echo "    | <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    echo "</div>\n";
}

function pm_create_new($error = null) {
    page_head(tra("Send private message"));
    
    if (post_str("preview", true) == tra("Preview")) {
        $options = new output_options;
        echo "<div id=\"preview\">\n";
        echo "<div class=\"header\">".tra("Preview")."</div>\n";
        echo output_transform(post_str("content", true), $options);
        echo "</div>\n";
    }
    
    global $logged_in_user;
    $replyto = get_int("replyto", true);
    $userid = get_int("userid", true);
    
    $subject = null;
    $content = null;
    if ($replyto) {
        $message = BoincPrivateMessage::lookup_id($replyto);
        if (!$message || $message->userid != $logged_in_user->id) {
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
        $subject = stripslashes(post_str("subject", true));
        $content = stripslashes(post_str("content", true));
    }
    
    $content = htmlspecialchars($content);
    $subject = htmlspecialchars($subject);
    
    if ($error != null) {
        echo "<div class=\"error\">".$error."</div>\n";
    }
    
    echo "<form action=\"pm.php\" method=\"post\">\n";
    echo "<input type=\"hidden\" name=\"action\" value=\"send\">\n";
    echo form_tokens($logged_in_user->authenticator);
    start_table();
    row2(tra("To")."<br /><span class=\"smalltext\">".tra("User IDs or unique usernames, separated with commas")."</span>",
        "<input type=\"text\" name=\"to\" value=\"$writeto\" size=\"60\">"
    );
    row2(tra("Subject"), "<input type=\"text\" name=\"subject\" value=\"$subject\" size=\"60\">");
    row2(tra("Message")."<span class=\"smalltext\">".html_info()."</span>",
        "<textarea name=\"content\" rows=\"18\" cols=\"80\">$content</textarea>"
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

function pm_send($to_user, $subject, $content) {
    global $logged_in_user;
    $sql_subject = mysql_real_escape_string($subject);
    $sql_content = mysql_real_escape_string($content);
    $mid = BoincPrivateMessage::insert("(userid, senderid, date, subject, content) VALUES ($to_user->id, $logged_in_user->id, UNIX_TIMESTAMP(), '$sql_subject', '$sql_content')");
    if (!$mid) {
        error_page("Couldn't create message");
    }
    // send email notification if needed
    //
    BoincForumPrefs::lookup($to_user);
    switch ($to_user->prefs->pm_notification) {
    case 0:
    case 2:
        break;
    case 1:
        send_pm_notification_email(
            $logged_in_user, $to_user, $subject, $content
        );
        break;
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
        return "
            <br><span class=note>For email notification,
            <a href=edit_forum_preferences_form.php>edit community prefs</a></span>
        ";
    }
    return "";
}

function pm_rss($notify, &$title, &$msg, &$url) {
    $pm = BoincPrivateMessage::lookup_id($notify->opaque);
    $from_user = BoincUser::lookup_id($pm->senderid);
    $title = "New private message";
    $msg = "$from_user->name sent you a private message; subject: $pm->subject";
    $url = URL_BASE."pm.php";
}

$cvs_version_tracker[]="\$Id: pm.inc 14019 2007-11-01 23:04:39Z davea $";
?>
