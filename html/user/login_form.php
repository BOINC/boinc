<?php
    require_once("util.inc");

    page_head("Log in");
    echo "<form method=post action=login_action.php>
        <table cellpadding=8>
        <tr><td align=right>
        Your account ID:
        </td><td>
        <input name=authenticator size=40>
        </td></tr>

        <tr><td align=right>
        <br>
        </td><td>
        <input type=submit value='Log in'>
        </td></tr>
        </table>";

    page_tail();
?>
