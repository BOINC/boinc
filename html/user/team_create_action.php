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
require_once("../inc/team.inc");

$user = get_logged_in_user();

$name = post_str("name", true); 
if (strlen($name) == 0) {
    error_page(tra("You must choose a non-blank team name"));
}

$new_team = lookup_team_name($name);
if ($new_team) {
    error_page(tra("A team named %1 already exists - try another name", htmlentities($name)));
}

$url = post_str("url", true);
$type = post_str("type", true);
$name_html = post_str("name_html", true);
$description = post_str("description", true);
$country = post_str("country", true);
if ($country == "") {
    $country = "International";
}

$new_team = make_team(
    $user->id, $name, $url, $type, $name_html, $description, $country
);

if ($new_team) {
    user_join_team($new_team, $user);
    Header("Location: team_display.php?teamid=$new_team->id");
} else {
    error_page(tra("Could not create team - please try later."));
}

?>
