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


$new_email_addr = process_user_text($HTTP_POST_VARS["new_email_addr"]);
$new_email_addr = strtolower($new_email_addr);
if (!is_valid_email_addr($new_email_addr)) {
    show_error("Invalid email address:
        you must enter a valid address of the form
        name@domain"
    );
}
$user = lookup_user_email_addr($new_email_addr);
if (!$user) {
    $user = lookup_user_munged_email($new_email_addr);
}
if ($user) {
    show_error("There's already an account with that email address.");
}

$country = $_POST["country"];
if (!is_valid_country($country)) {
    echo "bad country";
    exit();
}

$postal_code = strip_tags(process_user_text($_POST["postal_code"]));

$authenticator = random_string();
$cross_project_id = random_string();
$munged_email_addr = munge_email_addr($new_email_addr, $authenticator);
$query = sprintf(
   "insert into user (create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid, venue, url, send_email, show_hosts, cross_project_id) values(%d, '%s', '%s', '%s', '%s', '%s', 0, 0, 0, '$project_prefs', $teamid, 'home', '', 1, 1, '$cross_project_id')",
    time(),
    $munged_email_addr,
    $new_name,
    $authenticator,
    boinc_real_escape_string($country),
    $postal_code
);
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
Header("Location: account_created.php?email_addr=$new_email_addr");

?>
