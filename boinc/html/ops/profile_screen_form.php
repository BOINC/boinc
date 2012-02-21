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
        ." and uotd_time is null "
        ." and expavg_credit>1 "
        ." and (response1 <> '' or response2 <> '') "
        ." order by recommend desc limit 20"
    ;
}
$result = mysql_query($query);

$n = 0;
echo "<form action=profile_screen_action.php>
";
start_table();
$found = false;
while ($profile = mysql_fetch_object($result)) {
    $found = true;
    echo "<tr><td valign=top>";
    buttons($n);
    echo "
        <br>Name: $profile->name
        <br>recommends: $profile->recommend
        <br>rejects: $profile->reject
        <br>RAC: $profile->expavg_credit
        <br>
    ";
    echo "</td><td><table border=2> ";
    show_profile($profile, $g_logged_in_user, true);
    echo "</table></td></tr>\n";
    echo "<input type=\"hidden\" name=\"userid$n\" value=\"$profile->userid\">\n";
    $n++;
}

end_table();

if ($found) {
    echo "
        <input type=\"hidden\" name=\"n\" value=\"$n\">
        <input type=\"submit\" value=\"OK\">
    ";
} else {
    echo "No more profiles to screen.";
}

echo "
    </form>
";

admin_page_tail();
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
