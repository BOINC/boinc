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

function escape2($strin) {
    $strout = null;

    for ($i = 0; $i < strlen($strin); $i++) {
        $ord = ord($strin[$i]);

        if (($ord > 0 && $ord < 32) || ($ord >= 127)) {
            $strout .= "&amp;#{$ord};";
        } else {
            switch ($strin[$i]) {
            case '<': $strout .= '&lt;'; break;
            case '>': $strout .= '&gt;'; break;
            case '&': $strout .= '&amp;'; break;
            case '"': $strout .= '&quot;'; break;
            default: $strout .= $strin[$i]; }
        }
    }
    return $strout;
}

function escape($strin) {
    $dom = new DOMDocument('1.0');
    $element = $dom->createElement('Element');
    $element->appendChild(
        $dom->createTextNode($strin)
    );

    $dom->appendChild($element);
    $x = $dom->saveXml();
    $x = substr($x, 31);
    $x = substr($x, 0, -11);
    return $x;
}

function handle_team($team, $f) {
    echo "Team: $team->name\n";
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
   <name>".escape($team->name)."</name>
   <url>".escape($team->url)."</url>
   <type>$team->type</type>
   <name_html>".escape($team->name_html)."</name_html>
   <description>
".escape($team->description)."
    </description>
   <country>$team->country</country>
   <id>$team->id</id>
   <user_email_munged>$user_email_munged</user_email_munged>
   <user_name>".escape($user->name)."</user_name>
   <user_country>".escape($user->country)."</user_country>
   <user_postal_code>".escape($user->postal_code)."</user_postal_code>
   <user_url>".escape($user->url)."</user_url>
</team>
"
    );
}

function main() {
    echo "------------ Starting at ".time_str(time())."-------\n";
    $f = fopen("temp.xml", "w");
    $teams = BoincTeam::enum(null);
    fwrite($f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<teams>\n");
    foreach($teams as $team) {
        handle_team($team, $f);
    }
    fwrite($f, "</teams>\n");
    fclose($f);
    if (!rename("temp.xml", "/home/boincadm/boinc-site/boinc_teams.xml")) {
        echo "Rename failed\n";
    }
    echo "------------ Finished at ".time_str(time())."-------\n";
}

main();

?>
