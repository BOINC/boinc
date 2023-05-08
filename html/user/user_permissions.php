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
    row1("Current special users", 99);

    echo "<tr><th>User</th>";
    for ($i=0; $i<S_NFLAGS; $i++) {
        echo "<th>" . $special_user_bitfield[$i] . "</th>\n";
    }
    echo "<th> </th></tr>";

    $result = _mysql_query(
        "SELECT prefs.userid, prefs.special_user, user.id, user.name
        FROM forum_preferences as prefs, user
        WHERE CONVERT(special_user, DECIMAL) > 0 and prefs.userid=user.id"
    );
    while ($foo = _mysql_fetch_object($result)) {
        echo "<tr>
            <td>$foo->name ($foo->id)</td>
            <form action=\"user_permissions.php\" method=\"POST\">
            <input type=\"hidden\" name=\"userid\" value=\"$foo->userid\">
        ";
        for ($j=0; $j<S_NFLAGS; $j++) {
            $bit = substr($foo->special_user, $j, 1);
            $c = ($bit == 1)?"checked":"";
            echo "<td>
                <input type=\"checkbox\" name=\"role".$j."\" value=\"1\" $c>
                </td>
            ";
        }
        echo "<td><input class=\"btn btn-default\" type=\"submit\" value=\"Update\"></td>";
        echo "</form></tr>\n";
    }

    echo "
        <tr>
        <form action=\"user_permissions.php\" method=\"POST\">
        <td>Add User ID:<input type=\"text\" name=\"userid\" size=\"6\"></td>
    ";

    for ($j=0; $j<S_NFLAGS; $j++) {
        echo "<td>
            <input type=\"checkbox\" name=\"role".$j."\" value=\"1\">
            </td>
        ";
    }
    echo "<td>
        <input class=\"btn btn-default\" type=\"submit\" value=\"Update\">
        </td>
        </form>
        </tr>
    ";

    end_table();

    page_tail();
}

function user_permissions_action() {
    $bitset = '';

    for ($i=0; $i<S_NFLAGS; $i++) {
        if (post_int("role".$i, TRUE) == 1) {
            $bitset .= '1';
            echo "<br> setting $i";
        } else {
            $bitset .= '0';
        }
    }
    $userid = post_int("userid");

    $query = "UPDATE forum_preferences SET special_user='$bitset' WHERE userid=$userid";
    _mysql_query($query);

    Header("Location: user_permissions.php");
}

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
if (!is_moderator($user, null)) {
    error_page("no access");
}

if (post_int("userid", true)) {
    user_permissions_action();
} else {
    user_permissions_form();
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
