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

require_once("../inc/util.inc");
require_once("../inc/email.inc");

check_get_args(array());

$user = get_logged_in_user();

page_head(tra("Change email address"));
echo tra("Note: if you change your email address, your %1weak account key%2 will change.", "<a href=weak_auth.php>", "</a>");
echo "<p>";

$email_text = "";
if (is_valid_email_syntax($user->email_addr)) {
    $email_text = $user->email_addr;
}

if ($user->email_addr_change_time + 604800 > time()) {
    echo tra("Email address was changed within the past 7 days. Please look for an email to $user->previous_email_addr if you need to revert this change.");
} else {
    form_start(secure_url_base()."edit_email_action.php", "post");
    form_input_text(
        tra("New email address").
        "<br><p class=\"text-muted\">".tra("Must be a valid address of the form 'name@domain'")."</p>",
        "email_addr", $email_text
    );

    // we need the password here not for verification,
    // but because we store it salted with email address,
    // which is about to change.

    form_input_text(tra("Password"), "passwd", "", "password");
    form_submit(tra("Change email address"));
    form_end();
}
page_tail();

?>
