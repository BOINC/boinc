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

require_once("../inc/db.inc");
require_once("../inc/util.inc");

check_get_args(array("next_url"));

$next_url = sanitize_local_url(get_str('next_url', true));

$u = "login_form.php?next_url=".urlencode($next_url);
redirect_to_secure_url($u);

$user = get_logged_in_user(false);

page_head(tra("Log in"));

if (0) {
echo '
    <a href="openid_login.php?openid_identifier=https://www.google.com/accounts/o8/id"><img src=img/google-button.png></a>
    <a href="openid_login.php?openid_identifier=http://yahoo.com"><img src=img/yahoo-button.png></a>
    <br>
';
}

echo "
    <form name=\"f\" method=\"post\" action=\"".secure_url_base()."/login_action.php\">
    <input type=\"hidden\" name=\"next_url\" value=\"$next_url\">
";
start_table();
if (LDAP_HOST) {
    $x = "Email address or LDAP user name:";
} else {
    $x = tra("Email address:");
}
row2($x . '<br><p class="text-muted"><a href="get_passwd.php">'.tra("forgot email address?")."</a></p>",
    "<input name=email_addr type=\"text\" size=40 tabindex=1>"
);
row2(tra("Password:") . '<br><p class="text-muted"><a href="get_passwd.php">' . tra("forgot password?") . "</a></p>",
    '<input type="password" name="passwd" size="40" tabindex="2">'
);
row2(tra("Stay logged in"),
    '<input type="checkbox" name="stay_logged_in" checked>'
);
$x = urlencode($next_url);

$config = get_config();
if (parse_bool($config, "disable_account_creation")
    || parse_bool($config, "no_web_account_creation")
) {
    $create_acct = "";
} else {
    $create_acct = tra("or %1create an account%2.", "<a href=\"create_account_form.php?next_url=$x\">","</a>");
}

row2("",
    "<input class=\"btn btn-default\" type=\"submit\" name=\"mode\" value=\"".tra("Log in")."\" tabindex=\"3\"><br><br>". $create_acct
);
if ($user) {
    row1("Log out");
    row2("You are logged in as $user->name",
        "<a href=\"logout.php?".url_tokens($user->authenticator)."\">Log out</a>"
    );
}
end_table();
echo "
    </form>
    <script type=\"text/javascript\">
        document.f.email_addr.focus();
    </script>
";

page_tail();
?>
