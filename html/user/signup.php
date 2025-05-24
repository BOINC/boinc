<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// signup.php: form for creating an account.
// Goes to the download page after that.

require_once("../inc/util.inc");
require_once("../inc/user_util.inc");
require_once("../inc/account.inc");
require_once("../inc/recaptchalib.inc");

function join_form() {
    // Using invitation codes to restrict access?
    //
    if (defined('INVITE_CODES')) {
        form_input_text(
            sprintf('<span title="%s">%s</span>',
                tra("An invitation code is required to create an account."),
                tra("Invitation code")
            ),
            "invite_code"
        );
    }

    form_input_text(
        sprintf('<span title="%s">%s</span>',
            tra("Identifies you on this web site. Use your real name or a nickname."),
            tra("Choose screen name")
        ),
        "new_name"
    );
    form_input_text(
        sprintf('<span title="%s">%s</span>',
            tra("An address where you can receive emails."),
            tra("Your email address")
        ),
        "new_email_addr"
    );
    $min_passwd_length = parse_element(get_config(), "<min_passwd_length>");
    if (!$min_passwd_length) {
        $min_passwd_length = 6;
    }

    form_input_text(
        sprintf('<span title="%s">%s</span>',
            tra("Must be at least %1 characters", $min_passwd_length),
            tra("Choose password")
        ),
        "passwd", "", "password", 'id="passwd"',
        passwd_visible_checkbox("passwd")
    );
}

// move this somewhere else
//
function global_prefs_form() {
    form_radio_buttons(
        "Use of your computer",
        "preset",
        array(
            array('green', "Green - limit power consumption"),
            array('standard', "Standard"),
            array('max', "Maximum"),
        ),
        'standard'
    );
}
function show_join_form() {
    page_head(
        sprintf("%s %s", tra("Join"), PROJECT),
        null, null, null, boinc_recaptcha_get_head_extra()
    );
    form_start("signup.php", "post");
    form_input_hidden("action", "join");
    join_form();
    //global_prefs_form();
    if (recaptcha_public_key()) {
        form_general("", boinc_recaptcha_get_html(recaptcha_public_key()));
    }
    form_submit(tra("Join"));
    form_end();
    page_tail();
}

function join_action() {
    $user = validate_post_make_user();
    if (!$user) {
        error_page("Couldn't create user record");
    }
    $preset = post_str("preset", true);
    if ($preset) {
        $prefs = compute_prefs_xml($preset);
        $user->update("global_prefs='$prefs'");
    }
    Header("Location: download_software.php");
    send_cookie('auth', $user->authenticator, false);
}

$action = post_str('action', true);
if ($action == "join") {
    join_action();
} else {
    show_join_form();
}
?>
