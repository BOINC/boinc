#! /usr/bin/env php
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

// export_credit_by_app.php dir
// write compressed XML versions of the credit_user and credit_team tables in
// dir/user_work.gz and dir/team_work.gz
//
// This is run by db_dump; you can also run it separately

require_once("../inc/util_ops.inc");

function export_item($item, $is_user, $f) {
    global $sub_projects;

    fprintf($f, $is_user?"<user>\n":"<team>\n");
    fprintf($f, "    <id>$item->id</id>\n");
    $crs = $is_user?
        BoincCreditUser::enum("userid=$item->id")
        : BoincCreditTeam::enum("teamid=$item->id")
    ;
    foreach ($sub_projects as $sub_project) {
        $total = 0;
        $average = 0;
        $njobs = 0;
        foreach ($crs as $cr) {
            if (in_array($cr->appid, $sub_project["appids"])) {
                $total += $cr->total;
                $average += $cr->expavg;
                $njobs += $cr->njobs;
            }
        }
        if ($total) {
            fprintf($f,
                "    <subproject name=\"".$sub_project["name"]."\">\n".
                "        <workunits>$njobs</workunits>\n".
                "        <credit>$total</credit>\n".
                "        <expavg_credit>$average</expavg_credit>\n".
                "    </subproject>\n"
            );
        }
    }

    fprintf($f, $is_user?"</user>\n":"</team>\n");
}

function export($is_user, $dir) {
    $n = 0;
    $filename = $is_user?"$dir/user_work":"$dir/team_work";
    $f = fopen($filename, "w");
    if (!$f) die("fopen");
    $is_user?  fprintf($f, "<users>\n"): fprintf($f, "<teams>\n");
    $maxid = $is_user?BoincUser::max("id"):BoincTeam::max("id");
    while ($n <= $maxid) {
        $m = $n + 1000;
        if ($is_user) {
            $items = BoincUser::enum_fields("id", "id>=$n and id<$m and total_credit>0");
        } else {
            $items = BoincTeam::enum_fields("id", "id>=$n and id<$m and total_credit>0");
        }
        foreach ($items as $item) {
            export_item($item, $is_user, $f);
        }
        $n = $m;
    }
    $is_user?  fprintf($f, "</users>\n"): fprintf($f, "</teams>\n");
    fclose($f);
    system("gzip -f $filename");
}

if ($argc != 2) die("usage");
$dir = $argv[1];
export(true, $dir);
export(false, $dir);
?>
