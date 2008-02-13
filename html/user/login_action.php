<?php
require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/user.inc");

// check for email/password case
//
$email_addr = strtolower(process_user_text(post_str("email_addr", true)));
$passwd = stripslashes(post_str("passwd", true));

if ($email_addr && $passwd) {
    $user = lookup_user_email_addr($email_addr);
    if (!$user) {
        error_page("No account found with email address $email_addr");
    }
	if (substr($user->authenticator, 0, 1) == 'x'){
		//User has been bad so we are going to take away ability to post for awhile.
		error_page("This account has been administratively disabled.");
	}
    $passwd_hash = md5($passwd.$email_addr);
    if ($passwd_hash != $user->passwd_hash) {
        page_head("Login failed");
        echo "Login failed: Wrong password for $email_addr.
            <br>Use your browser's Back button to try again.
            <p>
            If you've forgotten your password, you can either
            <ul>
            <li> <a href=edit_passwd_form.php>Change your password</a>
                (requires account key).
                <p>
                or
            <li> <a href=get_passwd.php>Have your account key emailed to you</a>.
            </ul>
        ";
        page_tail();
        exit();
    }
    $authenticator = $user->authenticator;
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "home.php";
    Header("Location: $next_url");
    $perm = $_POST['stay_logged_in'];
    send_cookie('auth', $authenticator, $perm);
    exit();
}

// check for time/id/hash case.

$id = get_int('id', true);
$t = get_int('t', true);
$h = get_str('h', true);
if ($id && $t && $h) {
    $user = BoincUser::lookup_id($id);
    if (!$user) error_page("no such user");
    $x = $id.$user->authenticator.$t;
    $x = md5($x);
    $x = substr($x, 0, 16);
    if ($x != $h) error_page("bad hash");
    if (time() - $t > 86400) {
        error_page("Link has expired;
            go <a href=get_passwd.php>here</a> to
            get a new login link by email."
        );
    }
    send_cookie('auth', $user->authenticator, true);
    Header("Location: home.php");
    exit();
}

// check for account key case.
// see if key is in URL; if not then check for POST data
//
$authenticator = process_user_text(get_str("key", true));
if (!$authenticator) {
   $authenticator = process_user_text(post_str("authenticator", true));
}
if (!$authenticator) {
    error_page("You must supply an account key");
}

if (substr($user->authenticator, 0, 1) == 'x'){
	//User has been bad so we are going to take away ability to post for awhile.
	error_page("This account has been administratively disabled.");
}
$user = lookup_user_auth($authenticator);
if (!$user) {
    page_head("Login failed");
    echo "No such account.";
    page_tail();
} else {
    $next_url = $_POST["next_url"];
    if (strlen($next_url) == 0) $next_url = "home.php";
    Header("Location: $next_url");
    $perm = $_POST['stay_logged_in'];
    send_cookie('auth', $authenticator, $perm);
}
?>
