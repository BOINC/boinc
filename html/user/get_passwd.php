<?php

require_once("util.inc");
require_once("user.inc");

page_head("Account key");


    echo "<form method=post action=mail_passwd.php>\n";
    start_table();
    row1("Get your ".PROJECT." account key");
    row2("Email address","<input size=40 name=email_addr>");
    row2("", "<input type=submit value=Submit>");
    echo "</table></form>
        Your account key will be emailed to you.
        You should receive it in a few minutes.<p>
    ";

page_tail();

?>
