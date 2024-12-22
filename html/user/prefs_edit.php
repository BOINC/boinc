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
include_once("../inc/prefs.inc");
include_once("../inc/prefs_project.inc");

$user = get_logged_in_user();

$action = sanitize_tags(get_str("action", true));
$subset = sanitize_tags(get_str("subset"));
$venue = sanitize_tags(get_str("venue", true));
$columns = get_int("cols", true);
$c = $columns?"&cols=$columns":"";
check_subset($subset);
if ($action) {
    check_tokens($user->authenticator);
    if ($subset == "global") {
        $main_prefs = prefs_parse_global($user->global_prefs);
        if ($venue) $prefs = $main_prefs->$venue;
        else $prefs = $main_prefs;
        $error = prefs_global_parse_form($prefs);
        if ($error != false) {
            $title = tra("Edit %1 preferences", subset_name($subset));
            if ($venue) $title = "$title for $venue";
            page_head($title);

            echo PREFS_FORM_DESC1;
            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "edit", $subset, $venue, $user, $prefs, $columns, $error
            );
        } else {
            if ($venue) $main_prefs->$venue = $prefs;
            else $main_prefs = $prefs;
            global_prefs_update($user, $main_prefs);
            Header("Location: prefs.php?subset=$subset&updated=1$c");
        }
    } else {
        $main_prefs = prefs_parse_project($user->project_prefs);
        if ($venue) $prefs = $main_prefs->$venue;
        else $prefs = $main_prefs;

        $project_error = prefs_project_parse_form($prefs);
        $error = prefs_resource_parse_form($prefs);
        if ($error != false || $project_error != false) {
            $title = tra("Edit %1 preferences", subset_name($subset));
            if ($venue) $title = tra("%1 for %2", $title, $venue);
            page_head($title);

            echo PREFS_FORM_ERROR_DESC;

            print_prefs_form(
                "edit", $subset, $venue, $user, $prefs, $columns, $error,
                $project_error
            );
        } else {
            if ($venue) {
                $main_prefs->$venue = $prefs;
            } else {
                $main_prefs = $prefs;
                prefs_privacy_parse_form($user);
                prefs_consent_parse_update($user);
            }

            project_prefs_update($user, $main_prefs);

            if (!$venue) {
                venue_parse_form($user);
                $user->update("venue='$user->venue'");
            }
            Header("Location: prefs.php?subset=$subset&updated=1$c");
        }
    }
} else {
    $title = tra("Edit %1 preferences", subset_name($subset));
    if ($venue) $title = tra("%1 for %2", $title, $venue);
    page_head($title);
    checkbox_clicked_js();

    if ($subset == "global") {
        echo PREFS_FORM_DESC1;
        $prefs = prefs_parse_global($user->global_prefs);
        if ($venue) {
            $prefs = $prefs->$venue;
        }
    } else {
        $prefs = prefs_parse_project($user->project_prefs);
        if ($venue) {
            $prefs = $prefs->$venue;
        }
    }
    print_prefs_form("edit", $subset, $venue, $user, $prefs, $columns);
}
echo "<a href=prefs.php?subset=$subset$c>".tra("Back to preferences")."</a>\n";
page_tail();

?>
