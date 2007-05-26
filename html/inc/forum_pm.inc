<?php
$cvs_version_tracker[]="\$Id$";

function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"forum_pm.php?action=inbox\">Inbox</a>\n";
    echo "    | <a href=\"forum_pm.php?action=new\">Write</a>\n";
    echo "    | <a href=\"forum_index.php\">Forum index</a>\n";
    echo "</div>\n";
}

function pm_create_new($error = null) {
    page_head("Private messages : Create new");
    pm_header();
    
    global $logged_in_user;
    $replyto = get_int("replyto", true);
    $userid = get_int("userid", true);
    
    if ($replyto) {
        $message = mysql_query("SELECT * FROM private_messages WHERE userid=".$logged_in_user->id." AND id=$replyto");
        if ($message) {
            $message = mysql_fetch_object($message);
            $content = "[quote]".$message->content."[/quote]\n";
            $userid = $message->senderid;
            $user = get_user_from_id($userid);
            if ($user != null) {
                $writeto = $userid." (".$user->name.")";
            }
            $subject = $message->subject;
            if (substr($subject, 0, 3) != "re:") {
                $subject = "re: ".$subject;
            }
        }
    } elseif ($userid) {
        $user = get_user_from_id($userid);
        if ($user != null) {
            $writeto = $userid." (".$user->name.")";
        }
    } else {
        $writeto = post_str("to", true);
        $subject = post_str("subject", true);
        $content = post_str("content", true);
    }
    
    $subject = htmlspecialchars($subject);
    
    if ($error != null) {
        echo "<div class=\"error\">$error</div>\n";
    }
    
    echo "<form action=\"forum_pm.php\" method=\"post\">\n";
    echo "<input type=\"hidden\" name=\"action\" value=\"send\">\n";
    echo form_tokens($logged_in_user->authenticator);
    start_table();
    echo "<tr><th>To<br /><span class=\"smalltext\">User IDs or unique usernames, separated with commas</span></th>\n";
    echo "<td><input type=\"text\" name=\"to\" value=\"$writeto\" size=\"60\"></td></tr>\n";
    echo "<tr><th>Subject</th><td><input type=\"text\" name=\"subject\" value=\"$subject\" size=\"60\"></td></tr>\n";
    echo "<tr><th>Message<br /><span class=\"smalltext\">".html_info()."</span></th>\n";
    echo "<td><textarea name=\"content\" rows=\"18\" cols=\"80\">$content</textarea></td></tr>\n";
    echo "<tr><td></td><td><input type=\"submit\" value=\"Send message\"></td></tr>\n";
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
    mysql_query("INSERT INTO private_messages (userid, senderid, date, subject, content) VALUES ($userid, $senderid, UNIX_TIMESTAMP(), '$sql_subject', '$sql_content')");
    if ($to->send_email == 1) { // Send email notification
        $message  = "Dear ".$to->name.",\n\n";
        $message .= "You have received a new private message at ".PROJECT." from ".$logged_in_user->name.", entitled \"".$subject."\".\n\n";
        $message .= "To read the original version, respond to, or delete this message, you must log in here:\n";
        $message .= URL_BASE."forum_pm.php\n\n";
        $message .= "Do not reply to this message. To disable email notification, go to\n";
        $message .= URL_BASE."prefs.php?subset=project\n";
        $message .= "and change email notification settings.\n";
        
        send_email($to, "[".PROJECT."] Private message notification", $message);
    }
}

?>
