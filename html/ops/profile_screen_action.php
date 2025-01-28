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

$n = $_GET["n"];

db_init();

admin_page_head("screen profile action");
for ($i=0; $i<$n; $i++) {
    $y = "user".$i;
    $val = $_GET[$y];
    $x = "userid".$i;
    $userid = $_GET[$x];
    switch ($val) {
    case 1:
        _mysql_query("update profile set verification=1 where userid=$userid");
        echo "<br>$userid is accepted";
        break;
    case -1:
        _mysql_query("update profile set verification=-1 where userid=$userid");
        echo "<br>$userid is rejected";
        break;
    case 0:
        echo "<br>$userid is skipped";
        break;
    }
}

echo "
    <p>
    <a href=\"profile_screen_form.php\">next 20</a>
";

admin_page_tail();

?>
