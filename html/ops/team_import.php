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

// fetch a list of "BOINC-wide teams" and create or update them

$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/user_util.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");
require_once("../project/project.inc");
require_once("../inc/consent.inc");

if (defined('INVITE_CODES')) {
    echo "Account creation is protected by invitation codes, so not importing teams";
    exit;
}

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    echo "Account creation is disabled\n";
    exit;
}

// set the following to 1 to print queries but not do anything

$dry_run = 0;

function lookup_team_seti_id($seti_id) {
    return BoincTeam::lookup("seti_id=$seti_id");
}

function decode($x) {
    if (!$x) return $x;
    return html_entity_decode($x, ENT_COMPAT, 'UTF-8');
}

function parse_team($f) {
    $t = new stdClass();
    while ($s = fgets($f)) {
        if (strstr($s, '</team>')) {
            $t->name = decode($t->name);
            $t->url = decode($t->url);
            $t->name_html = decode($t->name_html);
            $t->description = decode($t->description);
            $t->user_name = decode($t->user_name);
            $t->user_country = decode($t->user_country);
            $t->user_postal_code = decode($t->user_postal_code);
            $t->user_url = decode($t->user_url);
            return $t;
        }
        else if (strstr($s, '<name>')) $t->name = parse_element($s, '<name>');
        else if (strstr($s, '<url>')) $t->url = parse_element($s, '<url>');
        else if (strstr($s, '<type>')) $t->type = parse_element($s, '<type>');
        else if (strstr($s, '<name_html>')) $t->name_html = parse_element($s, '<name_html>');
        else if (strstr($s, '<description>')) {
            $t->description = '';
            while ($s = fgets($f)) {
                if (strstr($s, '</description>')) break;
                $t->description .= $s;
            }
        }
        else if (strstr($s, '<country>')) $t->country = parse_element($s, '<country>');
        else if (strstr($s, '<id>')) $t->id = parse_element($s, '<id>');
        else if (strstr($s, '<user_email_munged>')) {
            $user_email_munged = parse_element($s, '<user_email_munged>');
            $t->user_email = str_rot13($user_email_munged);
        }
        else if (strstr($s, '<user_name>')) $t->user_name = parse_element($s, '<user_name>');
        else if (strstr($s, '<user_country>')) $t->user_country = parse_element($s, '<user_country>');
        else if (strstr($s, '<user_postal_code>')) $t->user_postal_code = parse_element($s, '<user_postal_code>');
        else if (strstr($s, '<user_url>')) $t->user_url = parse_element($s, '<user_url>');
    }
    return null;
}

function valid_team($t) {
    if (!$t->id) return false;
    if (!$t->name) return false;
    if (!$t->user_email) return false;
    if (!$t->user_name) return false;
    return true;
}

function update_team($t, $team, $user) {
    global $dry_run;
    if (
        trim($t->url) == $team->url
        && $t->type == $team->type
        && trim($t->name_html) == $team->name_html
        && trim($t->description) == $team->description
        && $t->country == $team->country
        && $t->id == $team->seti_id
    ) {
        echo "   no changes\n";
        return;
    }
    echo "   updating\n";
    $url = BoincDb::escape_string($t->url);
    $name_html = BoincDb::escape_string($t->name_html);
    $description = BoincDb::escape_string($t->description);
    $country = BoincDb::escape_string($t->country);
    $query = "url='$url', type=$t->type, name_html='$name_html', description='$description', country='$country', seti_id=$t->id";
    if ($dry_run) {
        echo "   update to team $team->id: $query\n";
        return;
    }
    $retval = $team->update($query);
    if (!$retval) {
        echo "   update failed: $query\n";
        exit;
    }
}

function insert_case($t, $user) {
    global $master_url;
    global $dry_run;
    if ($dry_run) {
        if (!$user) echo "   making user $t->user_email\n";
        echo "   making team $t->name\n";
        return;
    }
    $make_user = FALSE;
    if (!$user) {
        list($checkct, $ctid) = check_consent_type(CONSENT_TYPE_ENROLL);
        if ($checkct) {
            echo "   cannot make user when an consent to terms of use is required\n";
        }
        else {
            echo "   making user $t->user_email\n";
            $user = make_user($t->user_email, $t->user_name, random_string());
            if (!$user) {
                echo "   Can't make user $t->user_email\n";
                return;
            }
            $make_user = TRUE;
        }
    }
    echo "   making team $t->name\n";
    // if user was not created, set the userid of a team to be zero
    $myid = 0;
    if ($make_user) {
        $myid = $user->id;
    }
    $team = make_team(
        $myid, $t->name, $t->url, $t->type, $t->name_html,
        $t->description, $t->country
    );
    if (!$team) {
        echo "   Can't make team $t->id\n";
        echo BoincDb::error();
        echo "\n";
        exit;
    }
    $team->update("seti_id=$t->id");
    if ($user) {
        $user->update("teamid=$team->id");

        send_email($user, "Team created on ".PROJECT,
        "An instance of the BOINC-wide team '$t->name'
has been created on the project:
name: ".PROJECT."
URL: $master_url
"
        );
    }
}

// There are several cases for a given record:
// (note: "ID" means the ID coming from BOINC, stored locally in seti_id)
// insert case:
//      There's no team with given name; create one,
//      and create the user if needed
// update1 case:
//      There's a team with the given name and the given ID
//      and its founder has the right email address.
//      Update its parameters if any are different.
// update2 case:
//      There's a team with the given name and seti_id=0,
//      and its founder has the right email address.
//      Update its parameters if any are different,
//      and set its seti_id.
//      This handles the case where the team founder created the team
//      before this new system was run.
// conflict case:
//      There's a team with the given name,
//      and either it has the wrong ID
//      or its founder has a different email address.
//      Don't change anything.

// These semantics mean that:
// -    A BOINC team can't change its name via this mechanism.
//      This avoids pathological cases, e.g. if two teams swapped names,
//      the updates would always fail.
//      If a BOINC team wants to change its name,
//      it must do it manually everywhere.
// -    If a BOINC team changes its founder (or the founder changes email)
//      they'll have to make this change manually on all projects.
//      (this is better than a security vulnerability)
// -    This mechanism can't be used to update the founder's
//      account parameters on all projects

function handle_team($f) {
    $t = parse_team($f);
    if (!$t) {
        echo "Failed to parse team\n";
        return;
    }
    //print_r($t);
    //return;
    if (!valid_team($t)) {
        echo "Invalid team\n";
        return;
    }

    echo "Processing $t->name $t->user_email\n";
    $user = BoincUser::lookup_email_addr($t->user_email);
    $team = BoincTeam::lookup_name($t->name);
    if ($team) {
        if (!$user) {
            echo "   team exists but user $t->user_email doesn't\n";
            return;
        }
        if ($user->id != $team->userid) {
            echo "   team exists but is owned by a different user\n";
            return;
        }
        if ($team->seti_id) {
            if ($team->seti_id == $t->id) {
                echo "   case 1\n";
                update_team($t, $team, $user);      // update1 case
            } else {
                echo "   team exists but has wrong seti_id\n";
            }
        } else {
            $team2 = lookup_team_seti_id($t->id);
            if ($team2) {
                // update1 case
                echo "   case 2\n";
                update_team($t, $team2, $user);
            } else {
                // update2 case
                echo "   case 3\n";
                update_team($t, $team, $user);
            }
        }
    } else {
        $team = lookup_team_seti_id($t->id);
        if ($team) {
            echo "   A team with same ID but different name exists;\n";
            echo "   Please report this to $t->user_email;\n";
        } else {
            echo "   Adding team\n";
            insert_case($t, $user);
        }
    }
}

function main() {
    echo "------------ Starting at ".time_str(time())."-------\n";
    $f = fopen("http://boinc.berkeley.edu/boinc_teams.xml", "r");
    if (!$f) {
        echo "Can't get times file\n";
        exit;
    }
    while ($s = fgets($f)) {
        if (strstr($s, '<team>')) {
            handle_team($f);
        }
    }
    echo "------------ Finished at ".time_str(time())."-------\n";
}

db_init();
main();

?>
