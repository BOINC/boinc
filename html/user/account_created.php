<?php
include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");
include_once("../inc/translation.inc");

db_init();

$email_addr = get_str("email_addr", true);

if ($email_addr) {
    // here when this page is reached via create_account_action.php
    //
    page_head(tr(AC_READY_TITLE));
    echo "
        <h3>".tr(AC_READY_WELCOME)."</h3>
        <p>
    ";
    $email_addr = process_user_text($email_addr);
    email_sent_message($email_addr);
} else {
    // here when user followed link in account-confirm email
    //
    page_head(tr(AC_CREATED_TITLE));
}
echo "
    <form method=post action=login_action.php>
    <input type=hidden name=next_url value=account_setup.php>
    <table cellpadding=8>
    <tr><td align=right>
        ".tr(AC_READY_PASTE)."
    </td>
    <td><input name=authenticator size=40></td>
    </tr><tr>
    <td align=right>".tr(AC_READY_CLICK)."</td>
    <td><input type=submit value='OK'></td>
    </tr></table>
    </form>
";

page_tail();

?>
