<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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
require_once("../inc/account.inc");

check_get_args(array());

$user = get_logged_in_user();

page_head(tra("Change password"));
echo tra("Note: if you change your password, your %1weak account key%2 will change.", "<a href=weak_auth.php>", "</a>");
echo "<p>";


form_start(secure_url_base()."edit_passwd_action.php", "post");
form_input_hidden('token', make_login_token($user));
form_input_text(tra(
    "New password"),
    "passwd",
    "",
    "password",
    'id="passwd"',
    passwd_visible_checkbox("passwd")
);
form_submit(tra("Change password"));
form_end();
page_tail();
?>
