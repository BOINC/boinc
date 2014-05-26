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
require_once("../inc/profile.inc");

if (DISABLE_PROFILES) error_page("Profiles are disabled");

$user = get_logged_in_user();

$cmd = get_str("cmd", true);

if ($cmd == "delete") {
    $result = delete_profile($user);
    if (!$result) {
        error_page(tra("couldn't delete profile - please try again later"));
    }
    delete_user_pictures($user->id);
    page_head(tra("Delete Confirmation"));
    $user->update("has_profile=0");
    echo tra("Your profile has been deleted.")."<br>";
    page_tail();
    exit();
}

page_head(tra("Profile delete confirmation"));

echo "
    <h2>".tra("Are you sure?")."</h2><p>
    ".tra("Deleted profiles are gone forever and cannot be recovered --
you will have to start from scratch
if you want another profile in the future.")."
    <p>
    ".tra("If you're sure, click 'Yes'
to remove your profile from our database.")."
    <p>
";
    show_button("delete_profile.php?cmd=delete", tra("Yes"), tra("Delete my profile"));
    show_button("index.php", tra("No"), tra("Do not delete my profile"));
page_tail();

?>
