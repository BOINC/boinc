<?php

require_once("boinc_db.inc");

function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"pm.php?action=inbox\">".tra("Inbox")."</a>\n";
    echo "    | <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    echo "</div>\n";
}

function pm_create_new($error = null) {
    page_head(tra("Private messages")." : ".tra("Create new"));
    
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
    echo "<tr><th>".tra("To")."<br /><span class=\"smalltext\">".tra("User IDs or unique usernames, separated with commas")."</span></th>\n";
    echo "<td><input type=\"text\" name=\"to\" value=\"$writeto\" size=\"60\"></td></tr>\n";
    echo "<tr><th>".tra("Subject")."</th><td><input type=\"text\" name=\"subject\" value=\"$subject\" size=\"60\"></td></tr>\n";
    echo "<tr><th>".tra("Message")."<br /><span class=\"smalltext\">".html_info()."</span></th>\n";
    echo "<td><textarea name=\"content\" rows=\"18\" cols=\"80\">$content</textarea></td></tr>\n";
    echo "<tr><td></td><td><input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\"> <input type=\"submit\" value=\"".tra("Send message")."\"></td></tr>\n";
    end_table();
    
    page_tail();
    exit();
}

function pm_send($to, $subject, $content) {
    global $logged_in_user;
    $userid = $to->id;
    $senderid = $logged_in_user->id;
    $sql_subject = mysql_real_escape_string($subject);
    $sql_content = mysql_real_escape_string($content);
    $to_user = BoincUser::lookup_id($userid);
    BoincForumPrefs::lookup($to_user);
    $send_email = false;
    if ($to_user->prefs->pm_notification) $send_email = true;
    BoincPrivateMessage::insert("(userid, senderid, date, subject, content) VALUES ($userid, $senderid, UNIX_TIMESTAMP(), '$sql_subject', '$sql_content')");
    if ($send_email) { // Send email notification
        $message  = "Dear ".$to->name.",\n\n";
        $message .= "You have received a new private message at ".PROJECT." from ".$logged_in_user->name.", entitled \"".$subject."\".\n\n";
        $message .= "To read the original version, respond to, or delete this message, you must log in here:\n";
        $message .= URL_BASE."pm.php\n\n";
        $message .= "Do not reply to this message. To disable email notification, go to\n";
        $message .= URL_BASE."prefs.php?subset=project\n";
        $message .= "and change email notification settings.\n";
        
        send_email($to, "[".PROJECT."] Private message notification", $message);
    }
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
    
    $output .= "<a href=\"pm.php?action=inbox\">".tra("Inbox")."</a> ($unread ".tra("unread").")\n";
    $output .= "| <a href=\"pm.php?action=new\">".tra("Write")."</a>\n";
    return $output;
}

$cvs_version_tracker[]="\$Id: pm.inc 14019 2007-11-01 23:04:39Z davea $";
?>
