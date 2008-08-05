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
require_once("../inc/xml.inc");
require_once("../inc/team.inc");
require_once("../inc/team_types.inc");

xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$auth = get_str("account_key");
$user = lookup_user_auth($auth);
if (!$user) {
    xml_error(-136);
}

$name = $_GET["name"];
if (strlen($name) == 0) {
    xml_error(-1, "must set team name");
}

$url = get_str("url");
$type_name = get_str("type");  // textual
$type = team_type_num($type_name);
$name_html = get_str("name_html");
$description = get_str("description");
$country = get_str("country");
if ($country == "") {
    $country = "International";
}

// the following DB-escapes its args
//
$new_team = make_team(
    $user->id, $name, $url, $type, $name_html, $description, $country
);

if ($new_team) {
    user_join_team($new_team, $user);
    echo "<create_team_reply>
    <success/>
    <team_id>$new_team->id</team_id>
</create_team_reply>
";
} else {
    xml_error(-137, "could not create team");
}

?>
