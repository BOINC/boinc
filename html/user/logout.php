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

include_once("../inc/db.inc");
include_once("../inc/util.inc");

$user = get_logged_in_user();

if ($user) {
    check_tokens($user->authenticator);
    clear_cookie('auth');
    page_head("Logged out");
    echo "You are now logged out";
    page_tail();
} else {
    error_page("not logged in");
}

?>
