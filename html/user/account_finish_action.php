<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");

$auth = process_user_text(post_str("auth"));
$name = process_user_text(post_str("name"));

if (strlen($name)==0) {
    error_page("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    error_page("HTML tags not allowed in name");
}

$country = post_str("country");
if (!is_valid_country($country)) {
    error_page( "invalid country");
}

$postal_code = strip_tags(process_user_text(post_str("postal_code", true)));

$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) {
    error_page("no such user");
}
$retval = $user->update("name='$name', country='$country', postal_code='$postal_code'");
if (!$retval) {
    error_page("database error");
}

session_start();
$_SESSION["authenticator"] = $auth;
Header("Location: team_search.php");
setcookie('auth', $auth, time()+3600*24*365);
setcookie('init', "1", time()+3600*24*365);

?>
