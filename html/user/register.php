<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

// Show a registration form allowing user to create account
// or log in to existing account.
// When done, take them to a download page.
//
// Link to this from a "Join" button on home page

// DEPRECATED; use signup.php instead

require_once("../inc/util.inc");
require_once("../inc/account.inc");
require_once("../inc/recaptchalib.inc");

function reg_form() {
    $config = get_config();
    $disable_acct = parse_bool($config, "disable_account_creation");
    page_head("Register",  null, null, null, boinc_recaptcha_get_head_extra());
    echo "<h3>Create an account</h3>";
    form_start("create_account_action.php", "post");
    create_account_form(0, "download_software.php");
    if (recaptcha_public_key()) {
        form_general("", boinc_recaptcha_get_html(recaptcha_public_key()));
    }
    form_submit("Join");
    form_end();

    echo "<h3>If you already have an account, log in</h3>";
    login_form("download_software.php");
    echo "</td></tr>";
    page_tail();
}

reg_form();
?>
