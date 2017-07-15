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

require_once("../inc/email.inc");

error_page("This feature is deprecated");

$text = get_str('text');
$subject = get_str('subject');

$user = get_logged_in_user();

page_head(tra("Sending emails"));
$found = false;
for ($i=0; $i<6; $i++) {
    $e = get_str("e$i", true);
    if (!$e) continue;
    $found = true;
    if (function_exists("make_php_mailer")) {
        require_once("../inc/phpmailer/class.phpmailer.php");
        $mail = make_php_mailer();
        $mail->AddAddress($e);
        $mail->Subject = $subject;
        $mail->Body = $text;
        $mail->From = $user->email_addr;
        $mail->FromName = $user->name;
        if (!$mail->Send()) {
            echo "<br>".tra("failed to send email to %1: %2", $e, $mail->ErrorInfo)."\n";
            continue;
        }
    } else {
        if (!mail($e, $subject, $text, "From: $user->name <$user->email_addr>")) {
            echo "<br>".tra("failed to send email to %1", $e)."\n";
        }
    }
    echo "<br>".tra("email sent successfully to %1", $e)."\n";
}
if ($found) {
    echo "
        <p>".tra("Thanks for telling your friends about %1", PROJECT);
} else {
    echo tra("You forgot to enter email addresses; Please %1 return to the form %2 and enter them.", "<a href=ffmail_form.php>", "</a>");
}
page_tail();

?>
