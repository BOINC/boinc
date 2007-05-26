<?php
$cvs_version_tracker[]="\$Id$";

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
    
    if (get_int("sent", true) == 1) {
        echo "<div class=\"notice\">Your message has been sent.</div>\n";
    }
    
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
        echo "<tr><th>Sender</th><td>".user_links(get_user_from_id($message->senderid))." 
    <a href=\"forum_pm.php?action=block&amp;id=".$message->senderid."\"><img src=\"img/report_post.png\" width=\"9\" height=\"9\" alt=\"Block user\"></a></td></tr>";
        echo "<tr><th>Date</th><td>".time_str($message->date)."</td></tr>";
        echo "<tr><th>Message</th><td>".output_transform($message->content, $options)."</td></tr>";
        echo "<tr><td class=\"pm_footer\"></td><td>\n";
        echo "<a href=\"forum_pm.php?action=delete&amp;id=$id\">Delete</a>\n";
        echo " | <a href=\"forum_pm.php?action=new&amp;replyto=$id\">Reply</a>\n";
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
                } elseif ($user == -1) { // Non-unique username
                    pm_create_new("User $username is not unique; you will have to use user ID");
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
} elseif ($action == "block") {
    $id = get_int("id");
    $user = mysql_query("SELECT name FROM user WHERE id=$id");
    if ($user) {
        $user = mysql_fetch_object($user);
        page_head("Really block ".$user->name."?");
        echo "<div>Are you really sure you want to block user ".$user->name." from sending you private messages?<br>\n";
        echo "Please have in mind that you can only block a limited amount of users.</div>\n";
        echo "<div>Once the user has been blocked you can unblock it using forum preferences page.</div>\n";
        
        echo "<form action=\"forum_pm.php\" method=\"POST\">\n";
        echo form_tokens($logged_in_user->authenticator);
        echo "<input type=\"hidden\" name=\"action\" value=\"confirmedblock\">\n";
        echo "<input type=\"hidden\" name=\"id\" value=\"$id\">\n";
        echo "<input type=\"submit\" value=\"Add user to filter\">\n";
        echo "<a href=\"forum_pm.php?action=inbox\">No, cancel</a>\n";
        echo "</form>\n";
    } else {
        error_page("No such user");
    }
} elseif ($action == "confirmedblock") {
    check_tokens($logged_in_user->authenticator);
    $id = post_int("id");
    $user = new User($logged_in_user->id);
    $blocked = new User($id);
    $user->addIgnoredUser($blocked);

    page_head("User ".$blocked->getName()." blocked");
    
    echo "<div>User ".$blocked->getName()." has been blocked from sending you private messages. To unblock, visit
        <a href=\"edit_forum_preferences_form.php\">message board preferences</a>.</div>\n";
}

page_tail();

?>
