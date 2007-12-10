<?php

include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");

function show_error($str) {
    page_head("Can't update account");
    echo "$str<br>\n";
    echo BoincDb::error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

$auth = process_user_text(post_str("auth"));
$name = process_user_text(post_str("name"));

if (strlen($name)==0) {
    show_error("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    show_error("HTML tags not allowed in name");
}

$country = post_str("country");
if (!is_valid_country($country)) {
    show_error( "invalid country");
}

$postal_code = strip_tags(process_user_text(post_str("postal_code", true)));

$user = BoincUser::lookup("authenticator='$auth'");
if (!$user) {
    error_page("no such user");
}
$retval = $user->update("name='$name', country='$country', postal_code='$postal_code'");
if (!$retval) {
    show_error("database error");
}

session_start();
$_SESSION["authenticator"] = $auth;
Header("Location: team_search.php");
setcookie('auth', $auth, time()+3600*24*365);
setcookie('init', "1", time()+3600*24*365);

?>
