<?php

require_once('../include.php');
require_once('../util.inc');
require_once('forum.inc');

function subscribe($thread, $user=null) {
    if (!$thread) {
        show_result_page(false, NULL);
        exit();
    }
    
    if (!$user) {
        $user = get_logged_in_user(true);
    }
    $sql = "INSERT INTO subscriptions SET userid = " . $user->id . ", threadid = " . $thread->id;
    show_result_page(sql_query($sql), $thread);
}

function show_result_page($success, $thread) {
    if ($success) {
        doHeader("Subscription Successful");
        echo "<span class=\"title\">Subscription successful</span>";
        echo "<p>You are now subscribed to <b>", $thread->title, "</b>.  You will receive an email whenver another user posts to this thread.</p>";
    } else {
        // Chances are that an error of this kind will be caught at the DB level, but we'll add a case anyway.
        doHeader("Subscription Failed");
        echo "<span class=\"title\">Subscription failed</span>";
        if ($thread) {
            echo "<p>There was a problem subscribing you to ", $thread->title, ".  Please try again later.</p>";
        } else {
            echo "<p>The thread you tried to subscribe to does not exist.</p>";
        }
    }
    
    doFooter();
    
}

function notify_subscribers($thread) {
    $sql = "SELECT * FROM subscriptions WHERE threadid = " . $thread->id;
    $result = sql_query($sql);
    
    while ($row = mysql_fetch_assoc($result)) {
        send_notice_email($row['userid'], $thread);
    }
    
}

function send_notice_email($userId, $thread) {
    $result = sql_query("SELECT * FROM user WHERE id = $userId");
    $row = mysql_fetch_assoc($result);
    
    $title = PROJECT . ": A user has posted to your subscribed thread.";
    $body = "Another " . PROJECT . " user has posted to the thread \"" . $thread->title . "\".";
    // Include a link to the thread here.
           
    mail($row['email_addr'], $title, $body);
}

?>