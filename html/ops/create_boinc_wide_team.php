#! /usr/bin/env php

<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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


// Script to create a BOINC-wide team and corresponding account.
// Probably useful only to me.

// usage: create_boinc_wide_team.php username emailaddr teamname

require_once("../inc/user.inc");
require_once("../inc/team.inc");

if ($argc != 4) die("usage: create_boinc_wide_team username email teamname\n");

$user_name = $argv[1];
$email_addr = $argv[2];
$team_name = $argv[3];

$user = BoincUser::lookup_email_addr($email_addr);
if (!$user) {
    $passwd_hash = md5("foobar".$email_addr);
    $user = make_user($email_addr, $user_name, $passwd_hash);
    if (!$user) die("can't create user\n");

    echo "created user $user->id\n";
}

$team = make_team($user->id, $team_name, "", "", "", "", "");
if (!$team) die("can't create team\n");

echo "created team $team->id\n";

$retval = $user->update("email_validated=1, teamid=$team->id");
if (!$retval) {
    die("can't update user\n");
}

echo "done\n";
?>
