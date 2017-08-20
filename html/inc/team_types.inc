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

include_once("../inc/translation.inc");

$team_types = array(
    "",
    tra("Other"),
    tra("Company"),
    tra("Primary school"),
    tra("Secondary school"),
    tra("Junior college"),
    tra("University or department"),
    tra("Government agency"),
    tra("Non-profit organization"),
    tra("National"),
    tra("Local/regional"),
    tra("Computer type"),
    tra("Social/political/religious")
);

function team_type_name($num) {
    global $team_types;
    if ($num>0 && $num<count($team_types)) {
        return $team_types[$num];
    }
    return "None";
}

function is_valid_team_type($name) {
    global $team_types;
    foreach ($team_types as $type) {
        if ($name == $type) return true;
    }
    return false;
}

function team_type_num($name) {
    global $team_types;
    for ($i=0; $i<sizeof($team_types); $i++) {
        if ($name == $team_types[$i]) return $i;
    }
    return 0;
}

function team_type_select($selected_type, $allow_none=false){
    global $team_types;

    $types = $team_types;
    if ($allow_none) {
        $types[0] = '---';
        $type = 0;
    }
    return select_from_array('type', $types, $selected_type);
}

?>
