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
require_once("../inc/user.inc");
require_once("../inc/util.inc");
require_once("../inc/countries.inc");
require_once('../inc/recaptchalib.php');

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);

page_head(tra("Generate proof of account ownership"), null, null, null, boinc_recaptcha_get_head_extra());

// Verify the user is logged in
if ($user) {
    // If the following keys do not exist, then the users will be shown an error message.
    $config = get_config();
    $keydir = parse_config($config, "<key_dir>");
    $private_key_path = "file://$keydir/ownership_sign_private.pem";
    $public_key_path = "file://$keydir/ownership_sign_public.pem";

    // Check that the private key file exists where specified. If not, redirect to error page.
    if (!file_exists($private_key_path)) {
        error_page(tra("The private key doesn't exist. Contact the project administrator to resolve this issue."));
    }

    // Check that the public key file exists where specified. If not, redirect to error page.
    if (!file_exists($public_key_path)) {
        error_page(tra("The public key doesn't exist. Contact the project administrator to resolve this issue."));
    }

    echo "<p>This tool is designed to create a proof of account ownership for external systems.</p>";
    echo "<p>Enter a message with length less than 4096 characters into the input textbox below, solve the captcha then click the 'Generate' button.</p>";
    echo "<p>A textbox will then appear which contains your proof of account ownership.";
    echo "<form method=post action=openssl_sign_action.php>";

    echo form_tokens($user->authenticator);
    echo "<input name=user_data type=text size=20 value='Enter text'><br/><br/>";

    // Trigger recaptcha!
    global $recaptcha_public_key;
    if ($recaptcha_public_key) {
        form_general("", boinc_recaptcha_get_html($recaptcha_public_key));
    }

    echo "<br/><input class=\"btn btn-default\" type=submit value='".tra("Generate")."'>";
    echo "</form><br/><hr/>";
} else {
    // The user is not logged in!
    echo "<p>You need to be logged in to use this functionality.</p>";
}

page_tail();

?>
