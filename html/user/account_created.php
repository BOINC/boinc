<?php
    include_once("../inc/db.inc");
    include_once("../inc/util.inc");
    include_once("../inc/email.inc");

    $email_addr = $_GET["email_addr"];
    page_head("Account created");
    if ($email_addr) {
        echo "
            <h3>Congratulations - your ".PROJECT." account has been created</h3>
            <p>
        ";
        email_sent_message($email_addr);
    } else {
        echo "<h3>Activate your ".PROJECT." account</h3>\n";
    }
    echo "
        <form method=post action=login_action.php>
        <input type=hidden name=next_url value=account_setup.php>
        <table cellpadding=8>
        <tr><td align=right>Copy and paste the account ID here:</td>
        <td><input name=authenticator size=40></td>
        </tr><tr>
        <td align=right>and click here:</td>
        <td><input type=submit value='OK'></td>
        </tr></table>
        </form>
    ";

    page_tail();

?>
