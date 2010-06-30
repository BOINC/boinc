<?php

require_once("../inc/util.inc");

function rsc_name($t) {
    switch ($t) {
    case 2: return "CPU";
    case 3: return "nvidia GPU";
    case 4: return "ATI GPU";
    }
    return "Unknown";
}

function av_desc($gavid) {
    if ($gavid >= 1000000) {
        $appid = (int)($gavid/1000000);
        $app = BoincApp::lookup_id($appid);
        if (!$app) return "Anonymous platform, missing app";
        $rsc_type = $gavid % 1000000;
        $r = rsc_name($rsc_type);
        return "$app->user_friendly_name (anonymous platform, $r)";

    } else {
        $av = BoincAppVersion::lookup_id($gavid);
        if (!$av) return "Missing app version";
        $app = BoincApp::lookup_id($av->appid);
        if (!$app) return "Missing app";
        $pc = (strlen($av->plan_class))?"($av->plan_class)":"";
        $v = number_format($av->version_num/100, 2);
        return "$app->user_friendly_name $v $pc";
    }
}

function show_hav($hav) {
    row1(av_desc($hav->app_version_id));
    row2("Number of tasks completed", $hav->pfc_n);
    row2("Max tasks per day", $hav->max_jobs_per_day);
    row2("Number of tasks today", $hav->n_jobs_today);
    row2("Consecutive valid tasks", $hav->consecutive_valid);
    $x = number_format($hav->turnaround_avg/86400, 2);
    row2("Average turnaround time", "$x days");
}

$hostid = get_int('hostid');

$havs = BoincHostAppVersion::enum("host_id=$hostid");

page_head("Application info for host $hostid");
start_table();
foreach ($havs as $hav) {
    //if (!$hav->pfc_n) continue;
    show_hav($hav);
}
end_table();

page_tail();
?>
