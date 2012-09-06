<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// utility for studying the credit system

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

define('COBB_SCALE', 200/86400e9);

function gavid($avid, $appid) {
    if ($avid < 0) {
        return $appid*1000000 - $avid;
    }
    return $avid;
}

function show_result($r, $w) {
    $gavid = gavid($r->app_version_id, $w->appid);
    $hav = BoincHostAppVersion::lookup($r->hostid, $gavid);
    $av = BoincAppVersion::lookup_id($r->app_version_id);
    $raw_credit = $r->elapsed_time*$r->flops_estimate*COBB_SCALE;
    echo "<hr><pre>
Result ID: <a href=db_action.php?table=result&id=$r->id>$r->id</a>
Host: <a href=credit.php?hostid=$r->hostid&avid=$r->app_version_id>$r->hostid</a>
elapsed_time: $r->elapsed_time
flops_estimate: ".$r->flops_estimate/1e9."
app_version_id: $r->app_version_id
";
    if ($av) {
        $host_scale = $av->pfc_avg/$hav->pfc_avg;
        $av_scale = $av->pfc_scale;
        echo "app version scale: $av->pfc_scale\n";
    } else {
        $host_scale = 1;
        $av_scale = 1;
    }
    echo "host scale: $host_scale
Credit:
    Original: $r->claimed_credit
    Raw: $raw_credit
    Scaled: ".$raw_credit*$host_scale*$av_scale."
    Granted: $r->granted_credit
";
    if ($hav) {
        echo "Host app version:
    PFC avg: $hav->pfc_avg
";
    }
}

function handle_result($resultid) {
    $r = BoincResult::lookup_id($resultid);
    $w = BoincWorkunit::lookup_id($r->workunitid);
    $rs = BoincResult::enum("workunitid=$r->workunitid and validate_state=1");
    $app_version_ids = array();
    foreach ($rs as $r) {
        show_result($r, $w);
        $app_version_ids[] = $r->app_version_id;
    }
    $app_version_ids = array_unique($app_version_ids);
    foreach ($app_version_ids as $avid) {
        show_av($avid);
    }
}

function show_host_av($host_id, $av_id) {
    $hav = BoincHostAppVersion::lookup("host_id=$host_id and $app_version_id=$av_id");
    page_head("Host $host_id / app_version $av_id credit");
    echo "Results";
    $rs = BoincResult::enum("hostid=$hostid and app_version_id=$avid and validate_state=1 order by id desc limit 100");
    start_table();
    table_header("Workunit", "Elapsed", "Proj FLOPS", "Raw credit", "granted");
    $n = 0;
    $total=0;
    foreach($rs as $r) {
        $raw_credit = $r->elapsed_time*$r->flops_estimate*COBB_SCALE;
        $n++;
        $total += $raw_credit;
        table_row(
            "<a href=credit.php?wu_id=$r->workunitid>$r->workunitid</a>",
            $r->elapsed_time,
            $r->projected_flops,
            $raw_credit,
            $r->granted_credit
        );
    }

    start_table();
    row2("PFC nsamples", $hav->pfc_n);
    row2("PFC avg", $hav->pfc_avg);
    row2("Average raw credit", $total/$n);
    end_table();

    page_tail();
}

function av_string($av_id) {
    if ($av_id> 0) {
        $av = BoincAppVersion::lookup($av_id);
        $plat = BoincPlatform::lookup_id($av->platformid);
        $x = "<a href=credit.php?av_id=$av_id>$plat->name $av->plan_class</a>";
    } else {
        $x = "Anonymous platform";
    }
    return $x;
}

function show_workunit($wu_id) {
    page_head("Workunit credit");
    $wu = BoincWorkunit::lookup_id($wu_id);
    $app = BoincApp::lookup_id($wu->appid);
    start_table();
    row2("App", "<a href=credit.php?app_id=$app->id>$app->user_friendly_name</a>");
    end_table();
    echo "Results";
    start_table();
    table_header("Host", "App version", "Elapsed", "FLOPS est");
    $results = BoincResult::enum("workunitid=$wu->id and validate_state=1");
    foreach ($results as $r) {
        table_row(
            "<a href=credit.php?host_id=$r->hostid>$r->hostid</a>",
            av_string($r->app_version_id),
            $r->elapsed_time,
            $r->flops_estimate
        );
    }
    end_table();
    page_tail();
}

function show_av($av_id) {
    $av = BoincAppVersion::lookup_id($av_id);
    $app = BoincApp::lookup_id($av->appid);
    $plat = BoincPlatform::lookup_id($av->platformid);
    $av_desc = "$plat->name $av->plan_class";
    page_head("App version $av_desc credit");

    start_table();
    row2("PFC samples", $av->pfc_n);
    row2("PFC average", $av->pfc_avg);
    row2("PFC scale", $av->pfc_scale);
    row2("App", $app->user_friendly_name);
    end_table();

    $results = BoincResult::enum("app_version_id=$av_id and validate_state=1");
    start_table();
    table_header("Host/App_version", "Elapsed", "FLOPS est");
    foreach ($results as $r) {
        $avs = av_string($r->app_version_id);
        table_row(
            "<a href=credit.php?host_id=$r->hostid&a_id=$r->app_version_id> host $r->hostid AV $avs</a>",
            $r->elapsed_time,
            $r->flops_estimate
        );
    }
    end_table();
    page_tail();
}

function show_app($app_id) {
    $app = BoincApp::lookup_id($app_id);
    page_head("App $app->user_friendly_name credit");
    $avs = BoincAppVersion::enum("appid=$app_id and deprecated=0");
    start_table();
    table_header("platform/class/version", "PFC nsamples", "PFC avg", "PFC scale");
    foreach ($avs as $av) {
        $plat = BoincPlatform::lookup_id($av->platformid);
        table_row(
            "<a href=credit.php?av_id=$av->id>$plat->user_friendly_name $av->plan_class $av->version_num</a>",
            $av->pfc_n,
            $av->pfc_avg,
            $av->pfc_scale
        );
    }
    end_table();
    page_tail();
}

$wu_id = get_int("wu_id", true);
$host_id = get_int("host_id", true);
$av_id = get_int("av_id", true);
$app_id = get_int("app_id", true);
if ($wu_id) {
    show_workunit($wu_id);
    exit;
}
if ($host_id && $av_id) {
    show_host_av($host_id, $av_id);
    exit;
}
if ($av_id) {
    show_av($av_id);
    exit;
}
if ($app_id) {
    show_app($app_id);
    exit;
}

error_page("no command");

?>
