<?php

require_once("docutil.php");
require_once("help_funcs.php");
require_once("help_db.php");

$volid = $_GET['volid'];

$vol = vol_lookup($volid);

function is_valid_email_addr($addr) {
    $pattern = '/^([^@]+)@([^@\.]+)\.([^@]{2,})$/';
    $match = preg_match($pattern, $addr);
    return (bool) $match;
}

function show_info($vol) {
    $x = "<span class=note> Country: $vol->country\n";
    if ($vol->availability) {
        $x .= "<br>Usual hours: $vol->availability";
    }
    if ($vol->specialties) {
        $x .= "<br>Specialties: $vol->specialties";
    }
    if ($vol->projects) {
        $x .= "<br>Projects: $vol->projects";
    }
    if ($vol->lang2) {
        $x .= "<br>Primary language: $vol->lang1";
        $x .= "<br>Secondary language: $vol->lang2";
    }
    $x .= "</span><p>";
    return $x;
}

function live_contact($vol) {
    $skypeid = $vol->skypeid;
    echo "
    <font size=+2><b>Contact $vol->name on Skype</b></font>
    <script>
        if (navigator.userAgent.indexOf('MSIE') != -1) {
            document.write(
                \"<br>If requested, please enable ActiveX controls for this site.<br>\"
            );
        }
    </script>
    <p>
    <script type=\"text/javascript\" src=\"http://download.skype.com/share/skypebuttons/js/skypeCheck.js\"></script>
    ";
    if ($vol->voice_ok) {
        echo "<a href=skype:$skypeid?call onclick=\"return skypeCheck();\"><img align=top border=0 src=images/help/phone_icon_green.gif> Call (voice)</a>
        ";
    }
    if ($vol->text_ok) {
        echo "<p><a href=skype:$skypeid?chat onclick=\"return skypeCheck();\"><img align=top border=0 src=images/help/skype_chat_icon.png> Chat (text)</a>
        ";
    }

    echo "
    <p>
    <span class=note>Note: BOINC helpers are unpaid volunteers.
    Their advise is not endorsed by BOINC
    or the University of California.</span>
    <hr>
    ";
}

function show_rating($vol, $rating) {
    echo "
        If $vol->name has helped you, please give us your feedback:
        <form action=help_vol.php>
        <input type=hidden name=volid value=\"$vol->id\">
    ";
    list_start();
    list_item(
        "Rating<br><span class=note>Would you recommend $vol->name to people seeking help with BOINC?</span>",
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
        <p>
        <h2>Contact $vol->name by email</h2>
        <form action=help_vol.php>
        <input type=hidden name=volid value=\"$vol->id\">
    ";
    list_start();
    list_item(
        "Your email address",
        input("email_addr", "")
    );
    list_item("Subject<br><span class=note>Include 'BOINC' in the subject so $vol->name will know it's not spam</span>", input("subject", ""));
    list_item("Message<br><span class=note>
            
        Please include a detailed description of the problem
        you're experiencing.
        If possible, include the contents of BOINC's message log.
        </span>",
        textarea("message", "")
    );
    list_item("", "<input type=submit name=send_email value=OK>");
    list_end();
    echo "</form>
    ";
}

$send_email = $_GET['send_email'];
$rate = $_GET['rate'];
session_set_cookie_params(86400*365);
session_start();
$uid = session_id();

if ($send_email) {
    $volid = $_GET['volid'];
    $subject = stripslashes($_GET['subject']);
    $vol = vol_lookup($volid);
    if (!$vol || $vol->hide) {
        boinc_error_page("No such volunteer $volid");
    }
    $msg = stripslashes($_GET['message']);
    if (!$msg) {
        boinc_error_page("You must supply a message");
    }
    $body = "The following message was sent by a BOINC Help user.\n";
    $email_addr = $_GET['email_addr'];
    if (!is_valid_email_addr($email_addr)) {
        boinc_error_page("You must specify a valid email address");
    }
    $reply = "\r\nreply-to: $email_addr";
    $body .= "\n\n";
    $body .= $msg;
    if (!$subject) $subject = "BOINC Help request";
    mail($vol->email_addr, $subject, $body, "From: BOINC".$reply);
    page_head("Message sent");
    echo "Your message has been sent to $vol->name";
    page_tail();
} else if ($rate) {
    $volid = $_GET['volid'];
    $vol = vol_lookup($volid);
    if (!$vol) {
        boinc_error_page("No such volunteer $volid");
    }
    $x = $_GET['rating'];
    if ($x==null) {
        boinc_error_page("no rating given");
    }
    $rating = (int) $x;
    if ($rating < 0 || $rating > 5) {
        boinc_error_page("bad rating");
    }
    $comment = stripslashes($_GET['comment']);
    $r = null;
    $r->volunteerid = $volid;
    $r->rating = $rating;
    $r->timestamp = time();
    $r->comment = $comment;
    $r->auth = $uid;
    if ($uid) {
        $oldr = rating_lookup($r);
        if ($oldr) {
            $retval = rating_update($r);
            if ($retval) vol_update_rating($vol, $oldr, $r);
        } else {
            $retval = rating_insert($r);
            if ($retval) vol_new_rating($vol, $rating);
        }
    } else {
        $retval = rating_insert($r);
        if ($retval) vol_new_rating($vol, $rating);
    }
    if (!$retval) {
        echo mysql_error();
        boinc_error_page("database error");
    }
    page_head("Feedback recorded");
    echo "Your feedback has been recorded.  Thanks.
        <p>
        <a href=help.php>Return to BOINC Help</a>.
    ";
    page_tail();
} else {
    page_head("Help Volunteer: $vol->name");
    echo show_info($vol);
    $status = skype_status($vol->skypeid);
    if ($status != $vol->status) {
        $vol->status = $status;
        $vol->last_check = time();
        if (online($vol->status)) {
            $vol->last_online = time();
        }
        vol_update_status($vol);
    }
    $image = button_image($status);
    echo "
        <script type=\"text/javascript\" src=\"http://download.skype.com/share/skypebuttons/js/skypeCheck.js\"></script>
        <img src=images/help/$image><p>
    ";
    echo "<table class=box cellpadding=8 width=100%><tr><td width=40%>";
    if (online($status)) {
        live_contact($vol);
    }
    email_contact($vol);
    echo "</td></tr></table><p>\n";
    echo "<table class=box cellpadding=8 width=100%><tr><td>";
    $rating = rating_vol_auth($vol->id, $uid);
    if (!$rating) $rating->rating = -1;
    show_rating($vol, $rating);
    echo "</td></tr></table>\n";

    page_tail();
}
?>
