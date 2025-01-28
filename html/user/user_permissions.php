<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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

require_once('../inc/forum.inc');

db_init();

function user_permissions_form() {
    global $special_user_bitfield;
    page_head('Manage user privileges');

    start_table('table-striped');

    $x = ['User'];
    for ($i=0; $i<S_NFLAGS; $i++) {
        $x[] = $special_user_bitfield[$i];
    }
    $x[] = '';
    row_heading_array($x);

    $prefs = BoincForumPrefs::enum('CONVERT(special_user, DECIMAL) > 0');
    foreach ($prefs as $pref) {
        $user = BoincUser::lookup_id($pref->userid);
        echo '<form action="user_permissions.php" method="POST">';
        echo sprintf(
            '<input type="hidden" name="userid" value="%s">',
            $pref->userid
        );
        $x = ["$user->name ($user->id)"];
        for ($j=0; $j<S_NFLAGS; $j++) {
            $bit = substr($pref->special_user, $j, 1);
            $c = ($bit == 1)?"checked":"";
            $x[] = sprintf(
                '<input type="checkbox" name="role%d" value="1" %s>',
                $j, $c
            );
        }
        $x[] = '<input class="btn btn-success" type="submit" value="Update">';
        row_array($x);
        echo "</form>\n";
    }

    echo '<form action="user_permissions.php" method="POST">';
    $x = ['Add User ID: <input type="text" name="userid" size="6">'];
    for ($j=0; $j<S_NFLAGS; $j++) {
        $x[] = sprintf(
            '<input type="checkbox" name="role%d" value="1">',
            $j
        );
    }
    $x[] = "<input class=\"btn btn-success\" type=\"submit\" value=\"Update\">";
    row_array($x);
    echo "</form>\n";

    end_table();
    page_tail();
}

function user_permissions_action($user_id) {
    $bitset = '';
    $user = BoincUser::lookup_id($user_id);
    if (!$user) error_page('no user');
    BoincForumPrefs::lookup($user);

    for ($i=0; $i<S_NFLAGS; $i++) {
        if (post_int("role$i", true) == 1) {
            $bitset .= '1';
            echo "<br> setting $i";
        } else {
            $bitset .= '0';
        }
    }

    $user->prefs->update("special_user='$bitset'");
    Header("Location: user_permissions.php");
}

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
if (!is_admin($user)) {
    error_page("no access");
}

$user_id = post_int("userid", true);
if ($user_id) {
    user_permissions_action($user_id);
} else {
    user_permissions_form();
}

?>
