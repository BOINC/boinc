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

require_once("../inc/forum.inc");
require_once("../inc/text_transform.inc");
require_once("../inc/profile.inc");
require_once("../inc/util_ops.inc");
require_once("../project/project.inc");

db_init();

function buttons($i) {
    echo "
        <input type=\"radio\" name=\"user$i\" value=\"0\"> skip <br>
        <input type=\"radio\" name=\"user$i\" value=\"1\" checked=\"checked\"> accept <br>
        <input type=\"radio\" name=\"user$i\" value=\"-1\"> reject
    ";
}

admin_page_head("screen profiles");

if (function_exists('profile_screen_query')) {
    $query = profile_screen_query();
} else if (profile_screening()) {
    $query = "select * from profile, user where profile.userid=user.id "
        ." and has_picture>0 "
        ." and verification=0 "
        ." limit 20"
    ;
} else {
    $query = "select * from profile, user where profile.userid=user.id "
        ." and has_picture>0 "
        ." and verification=0 "
        ." and (uotd_time is null or uotd_time=0) "
        ." and expavg_credit>1 "
        ." and (response1 <> '' or response2 <> '') "
        ." order by recommend desc limit 20"
    ;
}
$result = _mysql_query($query);

$n = 0;
echo "<form action=profile_screen_action.php>
";
$found = false;
while ($profile = _mysql_fetch_object($result)) {
    $found = true;
    start_table();
    echo "<tr><td valign=top width=20%>";
    buttons($n);
    echo "
        <br>
        <br>Name: $profile->name
        <br>recommends: $profile->recommend
        <br>rejects: $profile->reject
        <br>RAC: ".format_credit($profile->expavg_credit)."
        <br>
    ";
    echo "</td><td>";
    start_table();
    show_profile($profile, $g_logged_in_user, true);
    end_table();
    echo "</td></tr><tr><td colspan=2></td></tr>\n";
    echo "<input type=\"hidden\" name=\"userid$n\" value=\"$profile->userid\">\n";
    $n++;
    end_table();
}


if ($found) {
    echo "
        <input type=\"hidden\" name=\"n\" value=\"$n\">
        <input class=\"btn btn-default\" type=\"submit\" value=\"OK\">
    ";
} else {
    echo "No more profiles to screen.";
}

echo "
    </form>
";

admin_page_tail();
?>
