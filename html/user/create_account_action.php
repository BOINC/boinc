<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");

function show_error($str) {
    page_head("Can't create account");
    echo "$str<br>\n";
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    page_head("Account creation is disabled");
    echo "
        <h3>Account creation is disabled</h3>
        Sorry, this project has disabled the creation of new accounts.
        Please try again later.
    ";
    exit();
}

db_init();
init_session();

$teamid = post_int("teamid", true);
if ($teamid) {
    $team = lookup_team($teamid);
    $clone_user = lookup_user_id($team->userid);
    if (!$clone_user) {
        echo "User $userid not found";
        exit();
    }
    $project_prefs = $clone_user->project_prefs;
} else {
    $teamid = 0;
    $project_prefs = "";
}

$new_name = process_user_text($_POST["new_name"]);
if (strlen($new_name)==0) {
    show_error("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    show_error("HTML tags not allowed in name");
}


$new_email_addr = process_user_text($_POST["new_email_addr"]);
$new_email_addr = strtolower($new_email_addr);
if (!is_valid_email_addr($new_email_addr)) {
    show_error("Invalid email address:
        you must enter a valid address of the form
        name@domain"
    );
}
$user = lookup_user_email_addr($new_email_addr);
if ($user) {
    show_error("There's already an account with that email address.");
}

$passwd = stripslashes(post_str("passwd"));
$passwd2 = stripslashes(post_str("passwd2"));
if ($passwd != $passwd2) {
    show_error("New passwords are different");
}

$min_passwd_length = parse_config($config, "<min_passwd_length>");
if (!$min_passwd_length) $min_passwd_length = 6;

if (!is_ascii($passwd)) {
    show_error("Passwords may only include ASCII characters.");
}

if (strlen($passwd)<$min_passwd_length) {
    show_error(
        "New password is too short:
        minimum password length is $min_passwd_length characters."
    );
}

$passwd_hash = md5($passwd.$new_email_addr);

$country = $_POST["country"];
if (!is_valid_country($country)) {
    echo "bad country";
    exit();
}

$postal_code = strip_tags(process_user_text($_POST["postal_code"]));

$authenticator = random_string();
$cross_project_id = random_string();
$country = boinc_real_escape_string($country);
$now = time();
$query = "insert into user (create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid, venue, url, send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$new_email_addr', '$new_name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, 'home', '', 1, 1, '$cross_project_id', '$passwd_hash')";
$result = mysql_query($query);
if (!$result) {
    show_error("Couldn't create account");
}

// In success case, redirect to a fixed page so that user can
// return to it without getting "Repost form data" stuff

$user->name = $new_name;
$user->email_addr = $new_email_addr;
$user->authenticator = $authenticator;
send_auth_email($user, true, false);

session_start();
$_SESSION["authenticator"] = $authenticator;
Header("Location: home.php?new_acct=1&via_web=1");
setcookie('auth', $authenticator, time()+3600*24*365);

?>
