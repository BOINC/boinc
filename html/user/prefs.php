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
require_once("../inc/prefs.inc");

db_init();

$user = get_logged_in_user();

$subset = get_str("subset");
$columns = get_int("cols", true);
$updated = get_int("updated", true);
$defaults = get_int("defaults", true);

page_head(tra("%1 preferences", subset_name($subset)));
if (isset($updated)) {
    echo "<p style='color: red'>
        ".tra("Your preferences have been updated, and
          will take effect when your computer communicates with %1
          or you issue the %2Update%3 command from the BOINC Manager.",
          PROJECT, "<strong>", "</strong>")."
        </p>
    ";
}
if (isset($defaults)) {
    echo "<p style='color: red'>
        ".tra("Your preferences have been reset to the defaults, and
          will take effect when your computer communicates with %1
          or you issue the %2Update%3 command from the BOINC Manager.",
          PROJECT, "<strong>", "</strong>")."
        </p>
    ";
}
if ($subset == "global") {
    print_prefs_display_global($user, $columns);
    if (!$defaults) {
        show_button(
            "prefs_default.php",
            "Restore defaults",
            "Restore default preferences"
        );
    }
} else {
    print_prefs_display_project($user, $columns);
}

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
