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

include_once("../inc/util.inc");
include_once("../inc/prefs.inc");
include_once("../inc/prefs_project.inc");

check_get_args(array("subset", "venue", "confirmed", "cols", "tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);

$subset = get_str("subset");
$venue = get_str("venue");
$confirmed = get_str("confirmed", true);
$columns = get_int("cols", true);
$c = $columns?"&cols=$columns":"";

if ($confirmed) {
    if ($subset == "global") {
        $main_prefs = prefs_parse_global($user->global_prefs);
        $main_prefs->$venue = null;
        global_prefs_update($user, $main_prefs);
    } else {
        $main_prefs = prefs_parse_project($user->project_prefs);
        $main_prefs->$venue = null;
        project_prefs_update($user, $main_prefs);
    }
    Header("Location: prefs.php?subset=$subset$c");
} else {
    page_head(tra("Confirm delete preferences"));
    $tokens = url_tokens($user->authenticator);
    echo "<p>".
        tra("Are you sure you want to delete your separate %1 preferences for %2?", subset_name($subset), $venue).
        "</p><br><br>\n";
    show_button(
        "prefs_remove.php?subset=$subset&venue=$venue&confirmed=yes$c$tokens",
        tra("Yes"), tra("Remove preferences")
    );
    show_button("prefs.php?subset=$subset$c", tra("Cancel"), tra("Cancel"));
    page_tail();
}

?>
