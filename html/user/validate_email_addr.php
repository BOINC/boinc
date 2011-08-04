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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");

function send_validate_email() {
    global $master_url;
    $user = get_logged_in_user();
    $x2 = uniqid(rand(), true);
    $user->update("signature='$x2'");
    send_email(
        $user,
        tra("Validate BOINC email address"),
        tra("Please visit the following link to validate the email address of your %1 account:", PROJECT)
        ."\n".$master_url."validate_email_addr.php?validate=1&u=$user->id&x=$x2"
    );
    page_head(tra("Validate email sent"));
    echo tra("An email has been sent to %1. Visit the link it contains to validate your email address.", $user->email_addr);
    page_tail();
}

function validate() {
    $x = get_str("x");
    $u = get_int("u");
    $user = lookup_user_id($u);
    if (!$user) {
        error_page(tra("No such user."));
    }

    $x2 = $user->signature;
    if ($x2 != $x) {
        error_page(tra("Error in URL data - can't validate email address"));
    }

    $result = $user->update("email_validated=1");
    if (!$result) {
        error_page(tra("Database update failed - please try again later."));
    }

    page_head(tra("Validate email address"));
    echo tra("The email address of your account has been validated.");
    page_tail();
}

if ($_GET['validate']) {
    validate();
} else {
    send_validate_email();
}

?>
