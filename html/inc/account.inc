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

// forms for account creation and login

function create_account_form($teamid, $next_url) {
    global $recaptcha_public_key;
    form_input_hidden('next_url', $next_url);

    if ($teamid) {
        form_input_hidden('teamid', $teamid);
    }

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
            tra("Identifies you on our web site. Use your real name or a nickname."),
            tra("Screen name")
        ),
        "new_name"
    );
    form_input_text(
        sprintf('<span title="%s">%s</span>',
            tra("Must be a valid address of the form 'name@domain'."),
            tra("Email address")
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
            tra("Password")
        ),
        "passwd", "", "password"
    );
    form_input_text(tra("Confirm password"), "passwd2", "", "password");
    form_select(
        sprintf('<span title="%s">%s</span>',
            tra("Select the country you want to represent, if any."),
            tra("Country")
        ),
        "country",
        country_select_options()
    );
    if (POSTAL_CODE) {
        form_input_text(
            tra("Postal or ZIP Code")."<br><small>".tra("Optional")."</small>",
            "postal_code"
        );
    }
}

function login_form($next_url) {
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
    if (!NO_COMPUTING) {
        $x .= '<br><small><a href="get_passwd.php">'.tra("forgot email address?")."</a></small>";
    }
    row2($x, '<input class="form-control" name=email_addr type="text" size=40 tabindex=1>');
    row2(tra("Password:") . '<br><small><a href="get_passwd.php">' . tra("forgot password?") . "</a></small>",
        '<input class="form-control" type="password" name="passwd" size="40" tabindex="2">'
    );
    row2(tra("Stay logged in"),
        '<input type="checkbox" name="stay_logged_in" checked>'
    );

    row2("",
        "<input class=\"btn btn-success\" type=\"submit\" name=\"mode\" value=\"".tra("Log in")."\" tabindex=\"3\"><br><br>"
    );

    end_table();
    echo "</form>\n";
}

?>
