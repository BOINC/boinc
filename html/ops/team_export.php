#!/usr/bin/env php

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

// This script for use ONLY by the BOINC-teams project.
// It generates an XML file with team and user info

$cli_only = true;
require_once("../inc/util_ops.inc");

function handle_team($team, $f) {
    $user = BoincUser::lookup_id($team->userid);
    if (!$user) {
        echo "no user for team $team->id\n";
        return;
    }
    if ($user->teamid != $team->id) {
        echo "Founder is not member of $team->name\n";
        return;
    }
    if (!$user->email_validated) {
        echo "the founder of $team->name, $user->email_addr, is not validated\n";
        return;
    }
    $user_email_munged = str_rot13($user->email_addr);
    fwrite($f, 
"<team>
   <name>".htmlspecialchars($team->name)."</name>
   <url>".htmlspecialchars($team->url)."</url>
   <type>$team->type</type>
   <name_html>".htmlspecialchars($team->name_html)."</name_html>
   <description>
".htmlspecialchars($team->description)."
    </description>
   <country>$team->country</country>
   <id>$team->id</id>
   <user_email_munged>$user_email_munged</user_email_munged>
   <user_name>".htmlspecialchars($user->name)."</user_name>
   <user_country>".htmlspecialchars($user->country)."</user_country>
   <user_postal_code>".htmlspecialchars($user->postal_code)."</user_postal_code>
   <user_url>".htmlspecialchars($user->url)."</user_url>
</team>
"
    );
}

function main() {
    $f = fopen("temp.xml", "w");
    $teams = BoincTeam::enum();
    fwrite($f, "<teams>\n");
    foreach($teams as $team) {
        handle_team($team, $f);
    }
    fwrite($f, "</teams>\n");
    fclose($f);
    if (!rename("temp.xml", "/home/boincadm/boinc/doc/boinc_teams.xml")) {
        echo "Rename failed\n";
    }
}

main();

?>
