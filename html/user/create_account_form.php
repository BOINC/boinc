<?php

require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');

// Web-based interface for account creation.
// This isn't needed for people who use the version 5 Manager's
// "attach project wizard".
// But (for the time being at least)
// it's needed for pre-version 5 clients,
// and clients that don't use the account manager

page_head(tra("Create an account"));

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    echo "
        <h1>".tra("Account creation is disabled")."</h1>
        <p>".tra("Account creation is currently disabled. Please try again later.")."</p>
    ";
    page_tail();
    exit();
}
$wac = parse_bool($config, "web_account_creation");
if (!$wac) {
    echo "<p>
        <b>".tra("NOTE: If you use BOINC version 5.2+ with the BOINC Manager, don't use this form. Just run BOINC, select Attach Project, and enter an email address and password.")."</b></p>
    ";
}

echo "
    <p>
    <form action=\"create_account_action.php\" method=\"post\">
";
$teamid = get_int("teamid", true);
if ($teamid) {
    $team = lookup_team($teamid);
    $user = lookup_user_id($team->userid);
    if (!$user) {
        echo "No such user";
    } else {
        echo "<b>".tra("This account will belong to the team %1 and will have the project preferences of its founder.", "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>")."</b><p>";
        echo "
            <input type=\"hidden\" name=\"teamid\" value=\"$teamid\">
        ";
    }
}
start_table();

// Using invitation codes to restrict access?
//
if(defined('INVITE_CODES')) {
     row2(
         tra("Invitation Code")."<br><span class=\"description\">".tra("A valid invitation code is required to create an account.")."</span>",
         "<input name=\"invite_code\" size=\"30\">"
     );
} 

row2(
    tra("Name")."<br><span class=\"description\">".tra("Identifies you on our web site. Use your real name or a nickname.")."</span>",
    "<input name=\"new_name\" size=\"30\">"
);
row2(
    tra("Email Address")."<br><span class=\"description\">".tra("Must be a valid address of the form 'name@domain'.")."</span>",
    "<input name=\"new_email_addr\" size=\"50\">"
);
$min_passwd_length = parse_element($config, "<min_passwd_length>");
if (!$min_passwd_length) {
    $min_passwd_length = 6;
}

row2(
    tra("Password")
    ."<br><span class=\"description\">".tra("Must be at least %1 characters", $min_passwd_length)."</span>",
    "<input type=\"password\" name=\"passwd\">"
);
row2(tra("Confirm password"), "<input type=\"password\" name=\"passwd2\">");
row2_init(
    tra("Country")."<br><span class=\"description\">".tra("Select the country you want to represent, if any.")."</span>",
    "<select name=\"country\">"
);
print_country_select();
echo "</select></td></tr>\n";
row2(
    tra("Postal or ZIP Code")."<br><span class=\"description\">".tra("Optional")."</span>",
    "<input name=\"postal_code\" size=\"20\">"
);
row2("",
    "<input type=\"submit\" value=\"".tra("Create account")."\">"
);
end_table();
echo "
    </form>
";

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
page_tail();
?>
