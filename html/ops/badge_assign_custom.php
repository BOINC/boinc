#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// Assign badges based on project total and per subproject credit.
// This code is mostly generic.
// You'll need to:
// - define your subproject in project/project.inc
// - use the <credit_by_app/> option in config.xml
// - supply your own project themed badge images
//  that have to follow a naming scheme (see below)
// See: https://github.com/BOINC/boinc/wiki/PerAppCredit

require_once("../inc/util_ops.inc");

// use $sub_projects defined in project/project.inc
// (this speeds up the assignment of badges)
// "total" is a special sub project and should only be defined here
//
global $sub_projects;
$badges_sub_projects = $sub_projects;
$badges_sub_projects[] = array("name" => "project total", "short_name" => "total");

// thresholds for the various badges
// currently we use the same threshold for all badges (total and subproject)
// minimum total credits for each level and corresponding names
//
$badge_levels = array(
    50000000, 100000000, 250000000, 500000000, 1000000000, 10000000000,
    250000000000, 500000000000
);
$badge_level_names = array(
    "50M", "100M", "250M", "500M", "1B", "10B", "25B", "50B"
);

// images located in html/user/img/ for each badge level
// the actual filename must have the subproject short name or "total" as prefix
// e.g.: A_bronze.png is the first level of subproject A,
// total_bronze.png is the first level of total credit (across all subprojects)
//
$badge_images = array(
    "_bronze.png", "_silver.png", "_gold.png", "_amethyst.png",
    "_turquoise.png", "_sapphire.png", "_ruby.png", "_emerald.png"
);

// consistency checks
//
$num_levels = count($badge_levels);
if ($num_levels <> count($badge_level_names)) {
    die("number of badge_levels is not equal to number of badge_level_names");
}
if ($num_levels <> count($badge_images)) {
    die("number of badge_levels is not equal to number of badge_images");
}

// get the record for a badge (either total or subproject)
// badge_name_prefix should be user or team
// sub_project is an array with name and short_name as in $sub_projects
//
function get_badges(
    $badge_name_prefix, $badge_level_names, $badge_images, $sub_project
) {
    $badges = array();
    $limit = count($badge_level_names);
    for ($i=0; $i < $limit; $i++) {
        $badges[$i] = get_badge($badge_name_prefix."_".$sub_project["short_name"]."_".$i, "$badge_level_names[$i] in ".$sub_project["name"]." credit", $sub_project["short_name"].$badge_images[$i]);
    }
    return $badges;
}

// decide which project total badge to assign, if any.
// Unassign other badges.
//
function assign_tot_badge($is_user, $item, $levels, $badges) {
    // count from highest to lowest level, so the user get's assigned the
    // highest possible level and the lower levels get removed
    //
    for ($i=count($levels)-1; $i>=0; $i--) {
        if ($item->total_credit >= $levels[$i]) {
            assign_badge($is_user, $item, $badges[$i]);
            unassign_badges($is_user, $item, $badges, $i);
            return;
        }
    }
    // no level could be assigned so remove them all
    //
    unassign_badges($is_user, $item, $badges, -1);
}

// decide which subproject badge to assign, if any.
// Unassign other badges.
//
function assign_sub_badge($is_user, $item, $levels, $badges, $where_clause) {
    if ($is_user) {
        $sub_total = BoincCreditUser::sum('total', "where userid=".$item->id." and ($where_clause)");
    } else {
        $sub_total = BoincCreditTeam::sum('total', "where teamid=".$item->id." and ($where_clause)");
    }
    // count from highest to lowest level, so the user get's assigned the
    // highest possible level and the lower levels get removed
    //
    for ($i=count($levels)-1; $i>=0; $i--) {
        if ($sub_total >= $levels[$i]) {
            assign_badge($is_user, $item, $badges[$i]);
            unassign_badges($is_user, $item, $badges, $i);
            return;
        }
    }
    // no level could be assigned so remove them all
    //
    unassign_badges($is_user, $item, $badges, -1);
}


// Scan through all the users/teams, 1000 at a time,
// and assign/unassign the badges (total and subproject)
//
function assign_all_badges(
    $is_user, $badge_levels, $badge_level_names, $badge_images,
    $subprojects_list
) {
    $kind = $is_user?"user":"team";

    // get badges for all subprojects including total
    //
    foreach ($subprojects_list as $sp) {
        $badges[$sp["short_name"]] = get_badges($kind, $badge_level_names, $badge_images, $sp);
    }

    $n = 0;
    $maxid = $is_user?BoincUser::max("id"):BoincTeam::max("id");
    while ($n <= $maxid) {
        $m = $n + 1000;
        if ($is_user) {
            $items = BoincUser::enum_fields("id, total_credit", "id>=$n and id<$m and total_credit>0");
        } else {
            $items = BoincTeam::enum_fields("id, total_credit", "id>=$n and id<$m and total_credit>0");
        }
        // for every user/team
        //
        foreach ($items as $item) {
            // for every subproject (incl. total)
            //
            foreach ($subprojects_list as $sp) {
                if ($sp["short_name"] == "total") {
                    assign_tot_badge($is_user, $item, $badge_levels, $badges["total"]);
                } else {
                    // appids come from project/project.inc
                    $where_clause = "appid in (". implode(',', $sp["appids"]) .")";
                    assign_sub_badge(
                        $is_user, $item, $badge_levels, $badges[$sp["short_name"]],
                        $where_clause
                    );
                }
            }
        }
        $n = $m;
    }
}

// one pass through DB for users
//
assign_all_badges(
    true, $badge_levels, $badge_level_names, $badge_images,
    $badges_sub_projects
);

// one pass through DB for teams
//
assign_all_badges(
    false, $badge_levels, $badge_level_names, $badge_images,
    $badges_sub_projects
);

?>
