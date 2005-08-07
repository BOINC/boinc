<?php
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

init_session();
db_init();

// First check for email/password case
$email_addr = strtolower(process_user_text(post_str("email_addr", true)));
$passwd = process_user_text(post_str("passwd", true));
if ($email_addr) {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account with email address $email_addr");
    }
    $passwd_hash = md5($passwd.$email_addr);
    if ($passwd_hash != $user->passwd_hash) {
        error_page("Bad password: $passwd_hash, $user->passwd_hash");
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
   $authenticator = process_user_text(post_str("authenticator"));
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
    // see if the account is unactivated (i.e. email address not verified).
    // If so activate it.

    if (split_munged_email_addr($user->email_addr, $authenticator, $email)) {
        if (!validate_user($user, $email)) {
            page_head("Account already exists");
            echo "
                We can't activate this account because
                an account with the same email address already exists.
                You should use this existing account.
                To get the key of this account,
                <a href=get_passwd.php>click here</a>.
            ";
            page_tail();
            exit();
        }

    }
    $_SESSION["authenticator"] = $authenticator;
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "home.php";
    Header("Location: $next_url");
    if ($_POST['send_cookie']) {
        setcookie('auth', $authenticator, time()+3600*24*365);
    }
}
?>
