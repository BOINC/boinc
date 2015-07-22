#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// Assign badges based on RAC percentile.
// Customize this to grant other types of badges

require_once("../inc/util_ops.inc");

// thresholds for the various badges
// (i.e. gold badge is for top 1% of active users/teams)
//
$badge_pctiles = array(1, 5, 25);
$badge_images = array("pct_1.png", "pct_5.png", "pct_25.png");

// get the records for percentile badges; create them if needed
//
function get_pct_badges($badge_name_prefix, $badge_pctiles, $badge_images) {
    $badges = array();
    for ($i=0; $i<3; $i++) {
        $badges[$i] = get_badge($badge_name_prefix."_".$i, "Top ".$badge_pctiles[$i]."% in average credit", $badge_images[$i]);
    }
    return $badges;
}

// get the RAC percentiles from the database
//
function get_percentiles($is_user, $badge_pctiles) {
    $percentiles = array();
    for ($i=0; $i<3; $i++) {
        if ($is_user) {
            $percentiles[$i] = BoincUser::percentile("expavg_credit", "expavg_credit>1", 100-$badge_pctiles[$i]);
        } else {
            $percentiles[$i] = BoincTeam::percentile("expavg_credit", "expavg_credit>1", 100-$badge_pctiles[$i]);
        }
        if ($percentiles[$i] === false) {
            die("Can't get percentiles\n");
        }
    }
    return $percentiles;
}

// decide which badge to assign, if any.
// Unassign other badges.
//
function assign_pct_badge($is_user, $item, $percentiles, $badges) {
    for ($i=0; $i<3; $i++) {
        if ($item->expavg_credit >= $percentiles[$i]) {
            assign_badge($is_user, $item, $badges[$i]);
            unassign_badges($is_user, $item, $badges, $i);
            return;
        }
    }
    unassign_badges($is_user, $item, $badges, -1);
}

// Scan through all the users/teams, 1000 at a time,
// and assign/unassign RAC badges
//
function assign_badges($is_user, $badge_pctiles, $badge_images) {
    $kind = $is_user?"user":"team";
    $badges = get_pct_badges($kind."_pct", $badge_pctiles, $badge_images);
    $pctiles = get_percentiles($is_user, $badge_pctiles);
    //echo "thresholds for $kind badges: $pctiles[0] $pctiles[1] $pctiles[2]\n";

    $n = 0;
    $maxid = $is_user?BoincUser::max("id"):BoincTeam::max("id");
    while ($n <= $maxid) {
        $m = $n + 1000;
        if ($is_user) {
            $items = BoincUser::enum_fields("id, expavg_credit", "id>=$n and id<$m and total_credit>0");
        } else {
            $items = BoincTeam::enum_fields("id, expavg_credit", "id>=$n and id<$m and total_credit>0");
        }
        foreach ($items as $item) {
            assign_pct_badge($is_user, $item, $pctiles, $badges);
            // ... assign other types of badges
        }
        $n = $m;
    }
}

echo "Starting: ", time_str(time()), "\n";

assign_badges(true, $badge_pctiles, $badge_images);
assign_badges(false, $badge_pctiles, $badge_images);

echo "Finished: ", time_str(time()), "\n";

?>
