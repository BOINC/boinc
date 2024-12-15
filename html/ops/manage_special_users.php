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

require_once('../inc/forum.inc');
require_once('../inc/util_ops.inc');

db_init();

admin_page_head('Manage user privileges');

start_table("align=\"center\"");
row1("Current special users", '9');

echo "<tr><td>User</td>";
for ($i=0; $i<S_NFLAGS; $i++) {
    echo "<td width=\"15\">" . $special_user_bitfield[$i] . "</td>\n";
}
echo "</tr>";

$result = _mysql_query(
    "SELECT prefs.userid, prefs.special_user, user.id, user.name
    FROM forum_preferences as prefs, user
    WHERE CONVERT(special_user, DECIMAL) > 0 and prefs.userid=user.id"
);
for ($i=1; $i<=_mysql_num_rows($result); $i++){
	$foo = _mysql_fetch_object($result);
    echo "<form action=\"manage_special_users_action.php\" method=\"POST\">\n";
    echo "<input type=\"hidden\" name=\"userid\" value=\"$foo->userid\"
        <tr><td>$foo->name ($foo->id)</td>
    ";
    for ($j=0; $j<S_NFLAGS; $j++) {
        $bit = substr($foo->special_user, $j, 1);
        echo "<td><input type=\"checkbox\" name=\"role".$j."\" value=\"1\"";
        if ($bit == 1) {
            echo " checked=\"checked\"";
        }
        echo "></td>\n";
    }
    echo "<td><input class=\"btn btn-default\" type=\"submit\" value=\"Update\"></form></td>";
    echo "</tr>\n";
}

echo "<tr><form action=\"manage_special_users_action.php\" method=\"POST\">\n";
echo "<td>Add UserID:<input type=\"text\" name=\"userid\" size=\"6\"></td>";

for ($j=0; $j<S_NFLAGS; $j++) {
    echo "<td><input type=\"checkbox\" name=\"role".$j."\" value=\"1\"";
    echo "></td>\n";
}
echo "<td><input class=\"btn btn-default\" type=\"submit\" value=\"Update\"></form></td>";
echo "</tr>\n";

end_table();

admin_page_tail();

?>
