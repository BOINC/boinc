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

require_once("../inc/util_ops.inc");
require_once("../inc/forum.inc");
require_once("../inc/profile.inc");

db_init();

/***********************************************************************\
 * Action: Process form info & controls
\***********************************************************************/

$limit = get_int('limit', true);
if (! $limit > 0 ) $limit = 30;



/***********************************************************************\
 * Display the page:
\***********************************************************************/

admin_page_head("New Users");

echo "<h2>Recently joined:</h2>\n";

echo "These are the most recent ".$limit." users to join the project.<br>\n";
echo "Clicking on a name opens a user management page <i>in another window or tab</i>\n";

echo "<form name=\"new_user_limit\" action=\"?\" method=\"GET\">\n";
echo "<label for=\"limit\">Limit displayed users to</label>\n";
echo "<input type=\"text\" value=\"".$limit."\" name=\"limit\" id=\"limit\" size=\"5\">";
echo "<input class=\"btn btn-default\" type=\"submit\" value=\"Display\">\n";
echo "</form>\n";

$query="SELECT * FROM user ORDER BY create_time DESC LIMIT $limit";
$result = _mysql_query($query);
if (_mysql_num_rows($result) < 1) {
    echo "There are no new users.";
    admin_page_tail();
}

start_table();
table_header("ID", "Name", "Email", "Team", "Country", "Joined");

while ($row = _mysql_fetch_object($result)) {
    $id = $row->id;
    $name = $row->name;
    $email = $row->email_addr;
    $country = $row->country;
    $joined = time_str($row->create_time);
    $email_validated = $row->email_validated;

    $team_name="";
    if($row->teamid > 0){
        $team = BoincTeam::lookup_id($row->teamid);
        $team_name = $team->name;
    }

    // Special Users:
    $roles = "";
    $user = $row;
    BoincForumPrefs::lookup($user);
    $special_bits = $user->prefs->special_user;
    if ($special_bits != "0") {
        for ($i = 0; $i < 7; $i++) {
            $bit = substr($special_bits, $i, 1);
            if ($bit == '1'){
                if (!empty($roles)) {
                    $roles .= ", ";
                }
                $roles .= $special_user_bitfield[$i];
            }
        }
    }
    if (!empty($roles)) {
        $roles = "<small>[$roles]</small>";
    }

    // Banished?
    if (!empty($user->banished_until)) {
        $dt = $user->banished_until - time();
        if( $dt > 0 ) {
            $x = "<span style=\"color: #ff0000\">Currently banished</span>";
        }
        else {
            $x = "<span style=\"color: #ff9900\">Previously banished</span>";
        }
        $roles .= $x;
    }

    if ($email_validated) {
        $email = "<span style=\"color: #ffff00\">".$email."</span>\n";
    } else {
        $email = "<span style=\"color: #ff0000\">".$email."</span>\n";
    }

    table_row($id, "<a href=\"manage_user.php?userid=".$id."\">".$name."</a> ".$roles, $email,
        $team_name, $country, $joined);
}
end_table();

admin_page_tail();

?>
