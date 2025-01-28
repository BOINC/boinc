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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/prefs.inc");

check_get_args(array("hostid", "venue"));

$user = get_logged_in_user();

$venue = get_str("venue");
check_venue($venue);
$hostid = get_int("hostid");

$host = BoincHost::lookup_id($hostid);
if (!$host) {
    error_page("No such computer");
}
if ($host->userid != $user->id) {
    error_page("Not your computer");
}

$retval = $host->update("venue='$venue'");
if ($retval) {
    page_head(tra("Computer venue updated"));
    if ($venue == '') {
        $venue = '('.tra("none").')';
    }
    echo sprintf(
        '%s
        <p>
        %s
        <p>
        <a class="btn" %s href=%s>%s</a>
        ',
        tra(
            'The venue of this computer has been set to %1.',
            "<b>$venue</b>"
        ),
        tra('Preference changes will take effect when the computer communicates with this project.'),
        button_style(),
        HOME_PAGE,
        tra('Continue to home page')
    );
    page_tail();
} else {
    db_error_page();
}

?>
