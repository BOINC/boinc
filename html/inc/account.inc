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
    echo "
        <form action=\"create_account_action.php\" method=\"post\">
        <input type=hidden name=next_url value=\"$next_url\">
    ";

    if ($teamid) {
        echo "
            <input type=\"hidden\" name=\"teamid\" value=\"$teamid\">
        ";
    }
    start_table();

    // Using invitation codes to restrict access?
    //
    if (defined('INVITE_CODES')) {
         row2(
             tra("Invitation Code")."<br><p class=\"text-info\">".tra("A valid invitation code is required to create an account.")."</p>",
             "<input type=\"text\" name=\"invite_code\" size=\"30\" >"
         );
    }

    row2(
        tra("Name")."<br><p class=\"text-info\">".tra("Identifies you on our web site. Use your real name or a nickname.")."</p>",
        "<input type=\"text\" name=\"new_name\" size=\"30\">"
    );
    row2(
        tra("Email Address")."<br><p class=\"text-info\">".tra("Must be a valid address of the form 'name@domain'.")."</p>",
        "<input type=\"text\" name=\"new_email_addr\" size=\"50\">"
    );
    $min_passwd_length = parse_element(get_config(), "<min_passwd_length>");
    if (!$min_passwd_length) {
        $min_passwd_length = 6;
    }

    row2(
        tra("Password")
        ."<br><p class=\"text-info\">".tra("Must be at least %1 characters", $min_passwd_length)."</p>",
        "<input type=\"password\" name=\"passwd\">"
    );
    row2(tra("Confirm password"), "<input type=\"password\" name=\"passwd2\">");
    row2_init(
        tra("Country")."<br><p class=\"text-info\">".tra("Select the country you want to represent, if any.")."</p>",
        "<select name=\"country\">"
    );
    print_country_select();
    echo "</select></td></tr>\n";
    if (0) {
        row2(
            tra("Postal or ZIP Code")."<br><p class=\"text-info\">".tra("Optional")."</p>",
            "<input type=\"text\" name=\"postal_code\" size=\"20\">"
        );
    }

    // Check if we're reCaptcha to prevent spam accounts
    //
    $publickey = parse_config(get_config(), "<recaptcha_public_key>");
    if ($publickey) {
        row2(
            "",
            recaptcha_get_html($publickey)
        );
    }

    row2("",
        "<input class=\"btn btn-primary\" type=\"submit\" value=\"".tra("Create account")."\">"
    );
    end_table();
    echo "</form>\n";
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
    row2($x . '<br><p class="text-muted"><a href="get_passwd.php">'.tra("forgot email address?")."</a></p>",
        "<input name=email_addr type=\"text\" size=40 tabindex=1>"
    );
    row2(tra("Password:") . '<br><p class="text-muted"><a href="get_passwd.php">' . tra("forgot password?") . "</a></p>",
        '<input type="password" name="passwd" size="40" tabindex="2">'
    );
    row2(tra("Stay logged in"),
        '<input type="checkbox" name="stay_logged_in" checked>'
    );

    row2("",
        "<input class=\"btn btn-default\" type=\"submit\" name=\"mode\" value=\"".tra("Log in")."\" tabindex=\"3\"><br><br>"
    );

    end_table();
    echo "</form>\n";
}

?>
