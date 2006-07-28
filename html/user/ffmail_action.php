<?php

require_once("../project/project.inc");
require_once("../inc/email.inc");
//$email_addr = get_str('email_addr');


function replace($toname, $comment, $fromname, $template) {
    $pat = array(
        '/<toname\/>/',
        '/<comment\/>/',
        '/<fromname\/>/',
    );
    $rep = array(
        $toname,
        $comment,
        $fromname,
    );
    return preg_replace($pat, $rep, $template);
}

$html = file_get_contents('../ops/ffmail/html');
$text = file_get_contents('../ops/ffmail/text');
$subject = file_get_contents('../ops/ffmail/subject');

$preview = get_str('preview', true);
$uname = get_str('uname');
$uemail = get_str('uemail');
$comment = get_str('comment', true);

$action = get_str('action');
if ($action=='Preview') {
    page_head('Email preview');
    echo "Your email will appear as follows:
        <hr>
    ";
    echo replace("[Friend's name]", $comment, $uname, $html);
    $args = strstr($_SERVER['REQUEST_URI'], '?');
    $args = str_replace('action=Preview', 'action=Send', $args);

    echo "
        <hr>
        <b><a href=ffmail_action.php$args>Send email</a></b>
        <p>
        [Use your browser's back button to return to message form]
    ";
    page_tail();
} else {
    page_head("Sending emails");
    $found = false;
    for ($i=0; $i<5; $i++) {
        $n = get_str("n$i", true);
        $e = get_str("e$i", true);
        if ($n && $e) {
            $found = true;
            $mail = new PHPMailer();
            $mail->AddAddress($e, $n);
            $mail->Subject = $subject;
            if ($html) {
                $mail->Body = replace($n, $comment, $uname, $html);
                $mail->AltBody = replace($n, $comment, $uname, $text);
            } else {
                $mail->Body = replace($n, $comment, $uname, $text);
            }
            $mail->From = $uemail;
            $mail->FromName = $uname;
            $mail->Host = $PHPMAILER_HOST;
            $mail->Mailer = $PHPMAILER_MAILER;
            if ($mail->Send()) {
                echo "<br>email sent successfully to $e\n";
            } else {
                echo "<br>failed to send email to $e: $mail->ErrorInfo\n";
            }
        }
    }
    if ($found) {
        echo "
            <p>
            Thanks for telling your friends about ".PROJECT.".
        ";
    } else {
        echo "
            You forgot to enter your friends' names and/or email addresses;
            Please <a href=ffmail_form.php>return to the form</a>
            and enter them.
        ";
    }
    page_tail();
}

exit();
?>
