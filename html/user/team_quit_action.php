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

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

check_get_args(array("tnow", "ttok"));

db_init();
$user = get_logged_in_user(true);
check_tokens($user->authenticator);
$teamid = post_int("id");
$team = lookup_team($teamid);
if ($user->teamid == $team->id) {
    user_quit_team($user);
    Header("Location: home.php");
} else {
    page_head(tra("Unable to quit team"));
    echo tra("Team doesn't exist, or you don't belong to it.");
}

page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
