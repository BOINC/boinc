<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once('../inc/db.inc');
require_once('../inc/util.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');
require_once('../inc/recaptchalib.php');

check_get_args(array("next_url", "teamid"));

$next_url = sanitize_local_url(get_str('next_url', true));

if (defined('SECURE_URL_BASE')
    && strstr(SECURE_URL_BASE, "https://")
    && !$_SERVER['HTTPS']
) {
    Header("Location: ".SECURE_URL_BASE."/create_account_form.php?next_url=$next_url");
    exit;

}

page_head(tra("Create an account"), null, null, null, IE_COMPAT_MODE);

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    echo "
        <h1>".tra("Account creation is disabled")."</h1>
        <p>".tra("Account creation is currently disabled. Please try again later.")."</p>
    ";
    page_tail();
    exit();
}

$nwac = parse_bool($config, "no_web_account_creation");
if (!$nwac && !no_computing()) {
    echo "<p>
        <b>".tra("NOTE: If you use the BOINC Manager, don't use this form. Just run BOINC, select Add Project, and enter an email address and password.")."</b></p>
    ";
}

echo "
    <p>
    <form action=\"create_account_action.php\" method=\"post\">
    <input type=hidden name=next_url value=\"$next_url\">
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
         "<input type=\"text\" name=\"invite_code\" size=\"30\" >"
     );
} 

row2(
    tra("Name")."<br><span class=\"description\">".tra("Identifies you on our web site. Use your real name or a nickname.")."</span>",
    "<input type=\"text\" name=\"new_name\" size=\"30\">"
);
row2(
    tra("Email Address")."<br><span class=\"description\">".tra("Must be a valid address of the form 'name@domain'.")."</span>",
    "<input type=\"text\" name=\"new_email_addr\" size=\"50\">"
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
    "<input type=\"text\" name=\"postal_code\" size=\"20\">"
);

// Check if we need reCaptcha for making more safe the creation of accounts
$publickey = parse_config($config, "<recaptcha_public_key>");

if ($publickey) {
    row2(
        tra("Please enter the words shown in the image"),
        recaptcha_get_html($publickey)
    );
}

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
