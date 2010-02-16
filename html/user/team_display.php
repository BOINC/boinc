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



require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/team.inc");

$teamid = get_int("teamid");
$team = BoincTeam::lookup_id($teamid);

$get_from_db = false;

$user = get_logged_in_user(false);

// always show fresh copy to admins; they might be editing info
//
if (is_team_admin($user, $team)) {
    $get_from_db = true;
}
if ($user->id == $team->ping_user) {
    $get_from_db = true;
}

// Cache the team record, its forum record, its new members,
// its admins, and its member counts

$cache_args = "teamid=$teamid";
if (!$get_from_db) {
    $cached_data = get_cached_data(TEAM_PAGE_TTL, $cache_args);
    if ($cached_data) {
        // We found some old but non-stale data, let's use it
        $team = unserialize($cached_data);
    } else {
        $get_from_db = true;
    }
}
if ($get_from_db) {
    $team->nusers = BoincUser::count("teamid=$teamid");
    $team->nusers_worked = BoincUser::count("teamid=$teamid and total_credit>0");
    $team->nusers_active = BoincUser::count("teamid=$teamid and expavg_credit>0.1");
    $team->forum = BoincForum::lookup("parent_type=1 and category=$team->id");
    $team->new_members = new_member_list($teamid);
    $team->admins = admin_list($teamid);
    $team->founder = BoincUser::lookup_id($team->userid);
    set_cache_data(serialize($team), $cache_args);
}

if (!$team) {
    error_page(tra("no such team"));
}

display_team_page($team, $user);

page_tail(true);

?>
