<?php

    include_once("util.inc");

    parse_str(getenv("QUERY_STRING"));
    page_head("Account created");
    echo "
        <h3>Congratulations - your ".PROJECT." account has been created</h3>
        <p>
        Your <b>account key</b> has been emailed to $email_addr.
        <br>
        Please wait until you receive this email (it may take a minute or two).
        <form method=post action=login_action.php>
        <input type=hidden name=next_url value=account_setup.php>
        <table cellpadding=8>
        <tr><td align=right>Then copy and paste the account key here:</td>
        <td><input name=authenticator size=40></td>
        </tr><tr>
        <td align=right>and click here:</td>
        <td><input type=submit value='OK'></td>
        </tr></table>
        </form>
        ";

    page_tail();

?>
