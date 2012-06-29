<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

require_once("../inc/util.inc");

check_get_args(array("hostid"));

function rsc_name($t) {
    switch ($t) {
    case 2: return tra("CPU");
    case 3: return tra("nvidia GPU");
    case 4: return tra("ATI GPU");
    }
    return tra("Unknown");
}

function av_desc($gavid) {
    if ($gavid >= 1000000) {
        $appid = (int)($gavid/1000000);
        $app = BoincApp::lookup_id($appid);
        if (!$app) return tra("Anonymous platform, missing app");
        $rsc_type = $gavid % 1000000;
        $r = rsc_name($rsc_type);
        return "$app->user_friendly_name (".tra("anonymous platform").", $r)";
    } else {
        $av = BoincAppVersion::lookup_id($gavid);
        if (!$av) return tra("Missing app version");
        $app = BoincApp::lookup_id($av->appid);
        if (!$app) return tra("Missing app");
        $platform = BoincPlatform::lookup_id($av->platformid);
        if (!$platform) return tra("Missing platform");
        $pc = (strlen($av->plan_class))?"($av->plan_class)":"";
        $v = number_format($av->version_num/100, 2);
        return "$app->user_friendly_name $v $platform->name $pc";
    }
}

function show_hav($hav) {
    row1(av_desc($hav->app_version_id));
    row2(tra("Number of tasks completed"), $hav->et_n);
    row2(tra("Max tasks per day"), $hav->max_jobs_per_day);
    row2(tra("Number of tasks today"), $hav->n_jobs_today);
    row2(tra("Consecutive valid tasks"), $hav->consecutive_valid);
    $x = number_format($hav->turnaround_avg/86400, 2);
    if ($hav->et_avg) {
        $gflops = 1e-9/$hav->et_avg;
        row2(tra("Average processing rate"), $gflops);
    }
    row2(tra("Average turnaround time"), "$x days");
}

$hostid = get_int('hostid');

$havs = BoincHostAppVersion::enum("host_id=$hostid");

page_head(tra("Application details for host %1", $hostid));
start_table();
foreach ($havs as $hav) {
    //if (!$hav->pfc_n) continue;
    show_hav($hav);
}
end_table();

page_tail();
?>
