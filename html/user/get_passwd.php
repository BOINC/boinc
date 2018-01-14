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

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");

check_get_args(array());

function show_email_form() {
    echo tra("Enter your account's email address below, and click OK. You will be sent email instructions for resetting your password.");

    echo "<p><p>";
    form_start("mail_passwd.php", "post");
    form_input_text(tra("Email address"), "email_addr");
    form_submit("OK");
}

page_head(tra("Reset password"));
show_email_form();
page_tail();

?>
