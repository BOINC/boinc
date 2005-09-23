<?php
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

init_session();
db_init();

$mode = post_str("mode");

// First check for email/password case
$email_addr = strtolower(process_user_text(post_str("email_addr", true)));
$passwd = stripslashes(post_str("passwd", true));
if ($mode == "Log in with email/password") {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account found with email address $email_addr");
    }
    $passwd_hash = md5($passwd.$email_addr);
    if ($passwd_hash != $user->passwd_hash) {
        page_head("Wrong password");
        echo "Wrong password for $email_addr.
            <p>
            If you've forgotten your password, you must
            <ul>
            <li> <a href=get_passwd.php>Have your account key emailed to you</a>.
            <li> <a href=edit_passwd_form.php>Change your password</a>.
            </ul>
        ";
        page_tail();
        exit();
    }
    $authenticator = $user->authenticator;
    $_SESSION["authenticator"] = $authenticator;
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "home.php";
    Header("Location: $next_url");
    if ($_POST['send_cookie']) {
        setcookie('auth', $authenticator, time()+3600*24*365);
    }
    exit();
}
// Now check for account key case.
// see if key is in URL; if not then check for POST data
//
$authenticator = process_user_text(get_str("key", true));
if (!$authenticator) {
   $authenticator = process_user_text(post_str("authenticator", true));
}
if (!$authenticator) {
    error_page("You must supply an account key");
}

$query = "select * from user where authenticator='$authenticator'";
$result = mysql_query($query);
if ($result) {
    $user = mysql_fetch_object($result);
    mysql_free_result($result);
}
if (!$user) {
    page_head("Log in");
    echo "
        We have no account with the key '$authenticator'.
        <br>Click <b>Back</b> to try again.
    ";
    page_tail();
} else {
    $_SESSION["authenticator"] = $authenticator;
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "home.php";
    Header("Location: $next_url");
    if ($_POST['send_cookie']) {
        setcookie('auth', $authenticator, time()+3600*24*365);
    }
}
?>
