<?php

require_once("util.inc");
require_once("user.inc");

page_head("Account key");
echo "<h2>Get your ".PROJECT." account key</h2>
     <p>
     <b>
     In order to protect your account from modification by others,
     we require both your account key for access.
     </b>
     <ul>
     <li>Do not give this account key out to anybody! If you get an
     e-mail requesting your account key, please ignore it - it's not from us.
     <li>The e-mail address you enter below is where we'll send the
     account key. It should be the same address you enter to log into
     ".PROJECT.".  If they aren't the same, we cannot send the account
     key for both security and administrative reasons.
     <li>If your BOINC login is not a valid email address, we
     unfortunately cannot help you access your account or your credits.
     You will need to create a new account with a valid e-mail address
     to access any future credit.
     </ul>
     <br clear=all>
     <table width=600 border=0 cellpadding=0 cellspacing=0><tr><td>

     <form method=post action=mail_passwd.php>
     Email address: <input name=email_addr>
     <input type=submit value=Submit>
     </form>
     </table>
     <p>
     Your account key will be sent by e-mail to the address you
     entered - you should receive it in a few minutes.<p>";

page_tail();

?>
