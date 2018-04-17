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

require_once("../inc/util.inc");
require_once("../inc/account.inc");

$user = get_logged_in_user();

$config = get_config();
if ( !parse_bool($config, "enable_delete_account") ) {
    error_page(
        tra("These feature is disabled.  Please contact the project administrator.")
    );
}

page_head(tra("Remove Account"));

echo "<p>".tra("You have the ability to delete your account.  Please note that this <b>cannot be undone</b> once it is completed.")."</p>"
    ."<p>".tra("The process works as follows:")."</p>"
    ."<ul>"
    ."<li>".tra("Enter in your password below and click on the \"Send Confirmation Email\" button")."</li>"
    ."<li>".tra("You will receive an email which contains a link.  Click on that link")."</li>"
    ."<li>".tra("On the page displayed, you will need to re-enter your password and then click \"Delete Account\"")."</li>"
    ."<li>".tra("Your account will then be immediately deleted")
    ."</ul><br/>";

form_start(secure_url_base()."delete_account_request_action.php", "post");
form_input_text(tra("Password"), "passwd", "", "password",'id="passwd"',passwd_visible_checkbox("passwd"));
form_submit(tra("Send Confirmation Email"));
form_end();

page_tail();
?>