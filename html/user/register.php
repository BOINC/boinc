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

// Show a registration form allowing user to create account
// or log in to existing account.
// When done, take them to a download page.
//
// Link to this from a "Join" button on home page

require_once("../inc/util.inc");
require_once("../inc/account.inc");

function reg_form() {
    $config = get_config();
    $disable_acct = parse_bool($config, "disable_account_creation");
    page_head("Register");
    start_table();
    echo "<tr><td>";
    echo "<h3>Create an account</h3>";
    create_account_form(0, "download.php");
    echo "</td><td>";
    echo "<h3>If you already have an account, log in</h3>";
    login_form("download.php");
    echo "</td></tr>";
    end_table();
    page_tail();
}

// if user is logged in, go straight to download page
//
$user = get_logged_in_user(false);
if ($user) {
    header("Location: download.php");
    exit;
}

// otherwise show registration form
//
reg_form();
?>
