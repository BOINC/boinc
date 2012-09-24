<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// reset user's preferences to the defaults

require_once("../inc/util.inc");
require_once("../inc/prefs.inc");

$confirmed = get_int("confirmed", true);
$user = get_logged_in_user();

if (!$confirmed) {
    page_head("Confirm reset preferences");
    echo "
        You have requested restoring your computing preferences
        to the default settings.
        Your current preferences will be discarded.
        Do you really want to do this?
        <br>
    ";
    show_button("prefs_default.php?confirmed=1", "Yes", "Restore default preferences");
    show_button("prefs.php", "No", "Keep current preferences");
    page_tail();
} else {
    $prefs = default_prefs_global();
    $retval = global_prefs_update($user, $prefs);
    if ($retval) {
        error_page("Couldn't restore default preferences.");
    } else {
        Header("Location: prefs.php?subset=global&defaults=1$c");
    }
}

?>
