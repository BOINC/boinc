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

// Agree to terms of use form, used for existing users who need to
// agree to the projects terms of use.

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/account.inc");

if (!isset($_COOKIE['tempuserid'])) {
    error_page(tra("You are not logged in. Please login to agree to our terms of use."));
}


check_get_args(array("next_url"));

$next_url = get_str('next_url', true);
$next_url = urldecode($next_url);
$next_url = sanitize_local_url($next_url);
$next_url = urlencode($next_url);

page_head(tra("Agree to our Terms of Use."));

user_agreetermsofuse_form($next_url);

page_tail();
?>