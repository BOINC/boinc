<?php

require_once("docutil.php");
require_once("help_funcs.php");
require_once("help_db.php");

$volid = $_GET['volid'];

$vol = vol_lookup($volid);

function live_contact($vol, $rating) {
    $skypeid = $vol->skypeid;
    echo "
    <h2>Contact $vol->name live using Skype</h2>
    <script type=\"text/javascript\" src=\"http://download.skype.com/share/skypebuttons/js/skypeCheck.js\"></script>
    ";
    if ($vol->voice_ok) {
        echo "<a href=skype:$skypeid?call onclick=\"return skypeCheck();\"><img align=top border=0 src=images/help/phone_icon_green.gif> Call $vol->name on Skype</a>
        ";
    }
    if ($vol->text_ok) {
        echo "<p><a href=skype:$skypeid?chat onclick=\"return skypeCheck();\"><img align=top border=0 src=images/help/skype_chat_icon.png> Chat with $vol->name on Skype</a>
        ";
    }

    echo "
    <hr>
    After the conversation is over, please give us your
    feedback:

    <form action=help_vol.php>
    <input type=hidden name=volid value=\"$vol->id\">

    ";
    list_start();
    list_item(
        "Rating<br><font size=-2>Would you recommend $vol->name to people seeking help with BOINC?</font>",
        star_select("rating", $rating->rating)
    );
    list_item("Comments", textarea("comment", $rating->comment));
    list_item("", "<input type=submit name=rate value=OK>");
    list_end();
    echo "
    </form>
    ";
}

function email_contact($vol) {
    echo "
        <h2>Contact $vol->name by email</h2>
        <form action=help_vol.php>
        <input type=hidden name=volid value=\"$vol->id\">
    ";
    list_start();
    list_item(
        "Your email address<br><font size=-2>Optional, but $vol->name
        won't be able to reply unless you include it</font>",
        input("email_addr", "")
    );
    list_item("Message", textarea("message", ""));
    list_item("", "<input type=submit name=send_email value=OK>");
    list_end();
    echo "</form>\n";
}

$send_email = $_GET['send_email'];
$rate = $_GET['rate'];
session_set_cookie_params(86400*365);
session_start();
$uid = session_id();

if ($send_email) {
    $volid = $_GET['volid'];
    $vol = vol_lookup($volid);
    if (!$vol) {
        error_page("No such volunteer $volid");
    }
    $msg = $_GET['message'];
    $body = "The following message was sent by a BOINC Help user.\n";
    $email_addr = $_GET['email_addr'];
    if ($email_addr) {
        $body .= "(email address: $email_addr)\n";
    }
    $body .= "\n\n";
    $body .= $msg;
    mail($vol->email_addr, "BOINC Help request", $body, "From: BOINC");
    page_head("Message sent");
    echo "Your message has been sent to $vol->name";
    page_tail();
} else if ($rate) {
    $volid = $_GET['volid'];
    $vol = vol_lookup($volid);
    if (!$vol) {
        error_page("No such volunteer $volid");
    }
    $x = $_GET['rating'];
    if (!$x) {
        error_page("no rating given");
    }
    $rating = (int) $x;
    if ($rating < 0 || $rating > 5) {
        error_page("bad rating");
    }
    $comment = stripslashes($_GET['comment']);
    $r = null;
    $r->volunteerid = $volid;
    $r->rating = $rating;
    $r->timestamp = time();
    $r->comment = $comment;
    $r->auth = $uid;
    if ($uid) {
        $retval = rating_update($r);
        if (!$retval) {
            $retval = rating_insert($r);
        }
    } else {
        $retval = rating_insert($r);
    }
    if (!$retval) {
        echo mysql_error();
        error_page("database error");
    }
    vol_update_rating($vol, $rating);
    page_head("Feedback recorded");
    echo "Your feedback has been recorded.  Thanks.
        <p>
        <a href=help.php>Return to BOINC Help</a>.
    ";
    page_tail();
} else {
    page_head("Contact $vol->name");
    $status = skype_status($vol->skypeid);
    $image = button_image($status);
    echo "
        <script type=\"text/javascript\" src=\"http://download.skype.com/share/skypebuttons/js/skypeCheck.js\"></script>
        <img src=images/help/$image><p>
    ";
    if (online($status)) {
        $rating = rating_vol_auth($vol, $uid);
        if (!$rating) $rating->rating = -1;
        live_contact($vol, $rating);
    }
    email_contact($vol);
    page_tail();
}
?>
