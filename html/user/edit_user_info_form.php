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
require_once("../inc/countries.inc");

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);

page_head(tra("Edit account information"));

form_start('edit_user_info_action.php', 'post');
echo form_tokens($user->authenticator);

form_input_text(
    tra("Screen name %1 real name or nickname%2", "<br><p class=\"small\">", "</p>"),
    'user_name',
    $user->name
);

if (USER_URL) {
    form_input_text(
        tra("URL %1 of your personal web page; optional%2", "<br><p class=\"small\">", "</p>"),
        'url',
        $user->url
    );
}

if (USER_COUNTRY) {
    form_select(
        tra("Country"),
        'country',
        country_select_options($user->country)
    );
}

if (POSTAL_CODE) {
    form_input_text(
        tra("Postal (ZIP) code %1 Optional%2", "<br><p class=\"small\">", "</p>"),
        'postal_code',
        $user->postal_code
    );
}

form_submit('Update info');
form_end();
page_tail();

?>
