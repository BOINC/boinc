<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

require_once("../inc/util.inc");
require_once("../inc/account.inc");
require_once("../inc/delete_account.inc");
require_once("../inc/user_util.inc");
require_once("../inc/email.inc");

$config = get_config();
if (!parse_bool($config, "enable_delete_account")) {
    error_page(
        tra("This feature is disabled.  Please contact the project administrator.")
    );
}

$user = get_logged_in_user();

if ($user->email_addr_change_time + 7*86400 > time()) {
    error_page(tra("You are not allowed to delete your account until after 7 days from when you last changed your email address."));
}

function delete_account_request_form($user) {
    page_head(tra("Delete Account"));

    echo "<p>".tra("You have the ability to delete your account.  Please note that this <b>cannot be undone</b> once it is completed.")."</p>"
        ."<p>".tra("The process works as follows:")."</p>"
        ."<ul>"
        ."<li>".tra("Enter in your password below and click on the \"Send Confirmation Email\" button")."</li>"
        ."<li>".tra("You will receive an email which contains a link.  Click on that link")."</li>"
        ."<li>".tra("On the page displayed, you will need to re-enter your password and then click \"Delete Account\"")."</li>"
        ."<li>".tra("Your account will then be immediately deleted")
        ."</ul><br/>";

    form_start(secure_url_base()."delete_account_request.php", "post");
    form_input_text(tra("Password"), "passwd", "", "password", 'id="passwd"', passwd_visible_checkbox("passwd"));
    form_submit(tra("Send Confirmation Email"));
    form_end();

    page_tail();
}

function delete_account_request_action($user) {
    $passwd = post_str("passwd");
    check_passwd_ui($user, $passwd);
    send_confirm_delete_email($user);

    page_head(tra("Confirmation Email Sent"));
    echo "<p>".tra("The email to confirm your request to delete your account has been sent.")."</p>";
    page_tail();
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    delete_account_request_action($user);
} else {
    delete_account_request_form($user);
}

?>
