<?php

require_once("util.inc");
require_once("user.inc");

page_head("Account key");
echo "<h2>Get your ".PROJECT." account key</h2>
     <p>
     <br clear=all>
     <table width=600 border=0 cellpadding=0 cellspacing=0><tr><td>

     <form method=post action=mail_passwd.php>
     Email address: <input name=email_addr>
     <input type=submit value=Submit>
     </form>
     </table>
     <p>
     Your account key will be emailed to this address.
     You should receive it in a few minutes.<p>";

page_tail();

?>
