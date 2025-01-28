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

require_once("../inc/util.inc");
require_once("../inc/prefs.inc");
require_once("../inc/prefs_project.inc");

$user = get_logged_in_user();
check_tokens($user->authenticator);

$action = sanitize_tags(get_str("action", true));
$subset = sanitize_tags(get_str("subset"));
$venue = sanitize_tags(get_str("venue"));
$columns = get_int("cols", true);
$c = $columns?"&cols=$columns":"";
check_venue($venue);
check_subset($subset);

if ($action) {
    if ($subset == "global") {
        $prefs = prefs_parse_global($user->global_prefs);
        $prefs->$venue = $prefs;

        $new_prefs = new stdClass();
        $error = prefs_global_parse_form($new_prefs);
        if ($error != false) {
            $title = tra("Edit %1 preferences", subset_name($subset));
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_DESC1;
            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "add", $subset, $venue, $user, $new_prefs, $columns, $error
            );
        } else {
            $prefs->$venue = $new_prefs;
            global_prefs_update($user, $prefs);
            Header("Location: prefs.php?subset=$subset$c");
        }
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        $prefs->$venue = $prefs;

        $new_prefs = new stdClass();
        $project_error = prefs_project_parse_form($new_prefs);
        $error = prefs_resource_parse_form($new_prefs);

        if ($error != false || $project_error != false) {
            $title = tra("Edit %1 preferences", subset_name($subset));
            if ($venue) $title = "$title for $venue";
            page_head($title);
            $x = $venue?"&venue=$venue":"";

            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "add", $subset, $venue, $user, $new_prefs, $columns,
                $error, $project_error
            );
        } else {
            $prefs->$venue = $new_prefs;
            project_prefs_update($user, $prefs);
            Header("Location: prefs.php?subset=$subset$c");
        }
    }
} else {
    $title = tra("Add %1 preferences for %2", subset_name($subset), $venue);
    page_head($title);

    if ($subset == "global") {
        $prefs = default_prefs_global();
        set_niu_prefs($prefs);
    } else {
        $prefs = default_prefs_project();
    }
    print_prefs_form("add", $subset, $venue, $user, $prefs, $columns);
}
page_tail();
?>
