<?php

require_once("util.inc");
require_once("user.inc");

page_head("Password");
echo "<h2>Get your BOINC password</h2>";
echo "<p>";
echo "<b>";
echo "In order to protect your account from modification by others, we require both your e-mail address and password for access. ";
echo "</b>";
echo "<ul>";
echo "<li>Do not give this password out to anybody! ";
echo "If you get an e-mail requesting ";
echo "your password, please ignore it - it's not from us. ";
echo "<li>The e-mail address you enter below is where we'll send the password. ";
echo "It should be the same address you enter to log into BOINC. ";
echo "If they aren't the same, we cannot send the password for both ";
echo "security and administrative reasons. ";
echo "<li>If your BOINC login is not a valid email address, ";
echo "we unfortunately cannot help you access your account or your credits. ";
echo "You will need to create a new account with a valid e-mail address ";
echo "to access any future credit. ";
echo "</ul>";
echo "<br clear=all>";
echo "<table width=600 border=0 cellpadding=0 cellspacing=0><tr><td>";

echo "<form action=mail_passwd.php>";
echo "Email address: <input name=email_addr>";
echo "<input type=submit value=Submit>";
echo "</form>";
echo "</table>";
echo "Your password will be sent by e-mail to the address you entered - ";
echo "you should receive it in a few minutes.";

page_tail();

?>
