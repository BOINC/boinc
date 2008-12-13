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
 
require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

$userid = get_int('userid');
$vote = get_str('vote');

$logged_in_user = get_logged_in_user();
if ($vote != "recommend" && $vote != "reject") {
    error_page(tra("Invalid vote type:")." ".htmlentities($vote));
}

BoincProfile::update_aux("$vote=$vote+1 WHERE userid = $userid");

page_head(tra("Vote Recorded"));

start_table_noborder();

row1(tra("Thank you"));

if ($vote == "recommend") {
    rowify(tra("Your recommendation has been recorded."));
} else {
    rowify(tra("Your vote to reject this profile has been recorded."));
}
end_table();
echo "<br><a href=\"view_profile.php?userid=", $userid ,"\">" . tra("Return to profile.") . "</a>";

page_tail();

?>
