<?php
$cvs_version_tracker[]="\$Id: forum_pm.php,v 1.119 2007/03/14 18:05:28 Rytis Exp $";

require_once("../inc/forum.inc");
require_once("../inc/forum_std.inc");
require_once("../inc/email.inc");
require_once("../inc/akismet.inc");

db_init();

$action = get_str("action", true);
if ($action == null) { $action = post_str("action", true); }
if ($action == null) { $action = "inbox"; }

$logged_in_user = get_logged_in_user();

if ($action == "inbox") {
    page_head("Private messages : Inbox");
    pm_header();

    $query = mysql_query("SELECT * FROM private_messages WHERE userid=".$logged_in_user->id." ORDER BY date DESC");
    if (mysql_num_rows($query) == 0) {
        echo "You have no private messages.";
    } else {
        start_table();
        print "<tr><th>Subject</th><th>Sender</th><th>Date</th></tr>\n";
        while ($row = mysql_fetch_object($query)) {
            print "<tr>\n";
            $subject = "<a href=\"forum_pm.php?action=read&id=".$row->id."\">".$row->subject."</a>";
            if ($row->opened) {
                print "<td>".$subject."</td>\n";
            } else {
                print "<td><strong>".$subject."</strong></td>\n";
            }
            print "<td>".user_links(get_user_from_id($row->senderid))."</td>\n";
            print "<td>".time_str($row->date)."</td>\n";
            print "</tr>\n";
        }
        end_table();
    }

} elseif ($action == "read") {
    $id = get_int("id");
    $message = mysql_query("SELECT * FROM private_messages WHERE id=".$id." AND userid=".$logged_in_user->id);
    if (mysql_num_rows($message) == 0) {
        error_page("No such message.");
    } else {
        $message = mysql_fetch_object($message);
        page_head("Private messages : ".$message->subject);
        pm_header();
        
        $options = new output_options;
        
        start_table();
        echo "<tr><th>Subject</th><td>".$message->subject."</td></tr>";
        echo "<tr><th>Sender</th><td>".user_links(get_user_from_id($message->senderid))."</td></tr>";
        echo "<tr><th>Date</th><td>".time_str($message->date)."</td></tr>";
        echo "<tr><th>Message</th><td>".output_transform($message->content, $options)."</td></tr>";
        echo "<tr><td class=\"pm_footer\"></td><td>\n";
        echo "<a href=\"forum_pm.php?action=delete&id=$id\">Delete</a>\n";
        echo " | <a href=\"forum_pm.php?action=new&replyto=$id\">Reply</a>\n";
        echo " | <a href=\"forum_pm.php?action=inbox\">Inbox</a>\n";
        end_table();
        
        if ($message->opened == 0) {
            mysql_query("UPDATE private_messages SET opened=1 WHERE id=$id");
        }
    }

} elseif ($action == "new") {
    pm_create_new();
} elseif ($action == "delete") {
    $id = get_int("id", true);
    if ($id == null) { $id = post_int("id"); }
    if (post_int("confirm", true) == 1) {
        check_tokens($logged_in_user->authenticator);
        mysql_query("DELETE FROM private_messages WHERE userid=".$logged_in_user->id." AND id=$id");
        header("Location: forum_pm.php");
    } else {
        $message = mysql_query("SELECT * FROM private_messages WHERE userid=".$logged_in_user->id." AND id=$id");
        if (mysql_num_rows($message) == 1) {
            $message = mysql_fetch_object($message);
            $sender = lookup_user_id($message->senderid);
            page_head("Private messages : Really delete?");
            pm_header();
            echo "<div>Are you sure you want to delete the message with subject \"".$message->subject."\" (sent by ".$sender->name." on ".time_str($message->date).")?</div>\n";
            echo "<form action=\"forum_pm.php\" method=\"post\">\n";
            echo form_tokens($logged_in_user->authenticator);
            echo "<input type=\"hidden\" name=\"action\" value=\"delete\">\n";
            echo "<input type=\"hidden\" name=\"confirm\" value=\"1\">\n";
            echo "<input type=\"hidden\" name=\"id\" value=\"$id\">\n";
            echo "<input type=\"submit\" value=\"Yes, delete\">\n";
            echo "</form>\n";
            echo "<form action=\"forum_pm.php\" method=\"post\">\n";
            echo "<input type=\"hidden\" name=\"action\" value=\"inbox\">\n";
            echo "<input type=\"submit\" value=\"No, cancel\">\n";
            echo "</form>\n";
        } else {
            error_page("No such message.");
        }
    }
} elseif ($action == "send") {
    check_tokens($logged_in_user->authenticator);

    $to = stripslashes(post_str("to", true));
    $subject = stripslashes(post_str("subject", true));
    $content = stripslashes(post_str("content", true));

    if (($to == null) || ($subject == null) || ($content == null)) {
        pm_create_new("You need to fill all fields to send a private message");
    } else {
        akismet_check(new User($logged_in_user->id), $content);
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
                    pm_create_new("Could not find user with id $userid");
                }
            } else {
                $user = lookup_user_name($username);
                if ($user == null) {
                    pm_create_new("Could not find user $username");
                }
            }
            $ignorelist = mysql_query("SELECT ignorelist FROM forum_preferences WHERE userid=".$user->id);
            $ignorelist = mysql_fetch_object($ignorelist);
            $ignorelist = $ignorelist->ignorelist;
            $ignorelist = explode("|", $ignorelist);
            if (in_array($logged_in_user->id, $ignorelist)) {
                pm_create_new("User ".$user->name." (ID: ".$user->id.") is not accepting private messages from you.");
            }
            if ($userids[$user->id] == null) {
                $userlist[] = $user;
                $userids[$user->id] = true;
            }
        }
        
        foreach ($userlist as $user) {
            pm_send($user, $subject, $content);
        }
        
        Header("Location: forum_pm.php?action=inbox&sent=1");
    }
}

page_tail();


function pm_header() {
    echo "<div>\n";
    echo "    <a href=\"forum_pm.php?action=inbox\">Inbox</a>\n";
    echo "    | <a href=\"forum_pm.php?action=new\">Write</a>\n";
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
