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

function delete_profile($user) {
    $result = BoincProfile::delete_aux("userid = $user->id");
    if (!$result) {
        error_page("couldn't delete profile - please try again later");
    }
    delete_user_pictures($user->id);
    page_head("Delete Confirmation");
    $user->update("has_profile=0");
    echo "Your profile has been deleted<br>";
    page_tail();
}

$user = get_logged_in_user();

if (isset($_POST['delete']) && $_POST['delete']) {
    delete_profile($user);
    exit();
}

page_head("Profile delete confirmation");

echo "<form action=", $_SERVER['PHP_SELF'], " method=\"POST\">";

echo "
    <h2>Are you sure?</h2><p>
    Deleted profiles are gone forever and cannot be recovered --
    you will have to start from scratch
    if you want another profile in the future.
    <p>
    If you're sure, click 'Delete'
    to remove your profile from our database.

    <p>
    <input type=submit name=delete value=Delete>
    </form>
";

page_tail();

?>
