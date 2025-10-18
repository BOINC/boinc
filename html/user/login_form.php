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
require_once("../inc/account.inc");

check_get_args(array("next_url"));

$next_url = get_str('next_url', true);
if ($next_url) {
    $next_url = urldecode($next_url);
    $next_url = sanitize_local_url($next_url);
    $next_url = urlencode($next_url);
}

$user = get_logged_in_user(false);
if ($user) {
    page_head(tra("Already logged in"));
    row2(tra("You are logged in as %1.", $user->name),
        sprintf(' <a href="logout.php?%s">%s</a>',
            url_tokens($user->authenticator),
            tra("Log out")
        )
    );
    page_tail();
    exit;
}

page_head(tra("Log in to ".PROJECT));

if (0) {
echo '
    <a href="openid_login.php?openid_identifier=https://www.google.com/accounts/o8/id"><img src=img/google-button.png></a>
    <a href="openid_login.php?openid_identifier=http://yahoo.com"><img src=img/yahoo-button.png></a>
    <br>
';
}

login_form($next_url);

$config = get_config();
if (!parse_bool($config, "disable_account_creation")
    && !parse_bool($config, "no_web_account_creation")
) {
    show_button(
        "create_account_form.php?next_url=$next_url",
        tra("Create account")
    );
}

page_tail();
?>
