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

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");

db_init();

admin_page_head("Manage special users action");

$bitset = '';

for ($i=0;$i<S_NFLAGS;$i++) {
    if (post_int("role".$i, TRUE) == '1') {
        $bitset = str_pad($bitset, $i+1, '1');
    } else {
        $bitset = str_pad($bitset, $i+1, '0');
    }
}
if ($bitset == "0000000") $bitset = '';
$userid = post_int("userid");

$query = "UPDATE forum_preferences SET special_user='$bitset' WHERE userid='$userid'";
_mysql_query($query);

if (_mysql_affected_rows() == 1) {
    echo "<center><h2>Success</h2>";
} else {
    echo "<center><h2>Failure</h2>";
}

echo "Query was: $query</center>";

//echo "<br><a href=\"manage_special_users.php\">Manage users</a>";

admin_page_tail();

?>
