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

// Assign badges based on RAC.
// Customize this to grant other types of badges

require_once("../inc/boinc_db.inc");

define("GOLD_RAC", 100000);
define("SILVER_RAC", 10000);
define("BRONZE_RAC", 1000);

function get_badge($name, $t, $rac, $image_url) {
    $b = BoincBadge::lookup("name='$name'");
    if ($b) return $b;
    $now = time();
    $title = "$t badge: average credit > $rac";
    $id = BoincBadge::insert("(create_time, name, title, image_url) values ($now, '$name', '$title', 'img/$image_url')");
    $b = BoincBadge::lookup_id($id);
    if ($b) return $b;
    die("can't create badge $name\n");
}

$rac_gold = get_badge("rac_gold", "Gold", GOLD_RAC, "gold.png");
$rac_silver = get_badge("rac_silver", "Silver", SILVER_RAC, "silver.png");
$rac_bronze = get_badge("rac_bronze", "Bronze", BRONZE_RAC, "bronze.png");

function assign_badge($user, $badge) {
    $now = time();
    $bbu = BoincBadgeUser::lookup("user_id=$user->id and badge_id=$badge->id");
    if ($bbu) {
        echo "reassigning $badge->name to $user->id\n";
        $bbu->update("reassign_time=$now where user_id=$user->id and badge_id=$badge->id");
    } else {
        echo "assigning $badge->name to $user->id\n";
        BoincBadgeUser::insert("(create_time, user_id, badge_id, reassign_time) values ($now, $user->id, $badge->id, $now)");
    }
}

function unassign_badges($user, $badges) {
    $list = null;
    foreach($badges as $badge) {
        echo "unassigning $badge->name to $user->id\n";
        if ($list) {
            $list .= ",$badge->id";
        } else {
            $list = "$badge->id";
        }
    }
    BoincBadgeUser::delete("user_id=$user->id and badge_id in ($list)");
}

function assign_rac_badge($user) {
    global $rac_gold, $rac_silver, $rac_bronze;
    if ($user->expavg_credit > GOLD_RAC) {
        assign_badge($user, $rac_gold);
        unassign_badges($user, array($rac_silver, $rac_bronze));
    } else if ($user->expavg_credit > SILVER_RAC) {
        assign_badge($user, $rac_silver);
        unassign_badges($user, array($rac_bronze, $rac_gold));
    } else if ($user->expavg_credit > BRONZE_RAC) {
        assign_badge($user, $rac_bronze);
        unassign_badges($user, array($rac_gold, $rac_silver));
    } else {
        unassign_badges($user, array($rac_gold, $rac_silver, $rac_bronze));
    }
}

function assign_badges_user($user) {
    assign_rac_badge($user);
    // ... assign other types of badges
}

function assign_badges() {
    $n = 0;
    $maxid = BoincUser::max("id");
    while ($n <= $maxid) {
        $m = $n + 1000;
        $users = BoincUser::enum_fields("id, expavg_credit", "id>=$n and id<$m and total_credit>0");
        foreach ($users as $user) {
            assign_badges_user($user);
        }
        $n = $m;
    }
}

assign_badges();

?>
