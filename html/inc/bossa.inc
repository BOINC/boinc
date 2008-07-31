<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

function bossa_job_create(
    $app_id, $batch_id, $info, $calibration
) {
    $job = new BossaJob;
    $info = serialize($info);
    if ($calibration) {
        $priority = drand();
        $c = 1;
    } else {
        $priority = 1;
        $c = 0;
    }

    $now = time();
    $int_max = 2147483647;
    $clause = "(create_time, app_id, batch_id, state, info, calibration, priority_0) values ($now, $app_id, $batch_id, 1, '$info', $c, $priority)";
    return $job->insert($clause);
}

function bossa_batch_create($appid, $name, $calibration) {
    $now = time();
    $c = $calibration?"1":"0";
    return BossaBatch::insert("(create_time, app_id, name, calibration) values ($now, $appid, '$name', $c)");
}

function bossa_app_lookup($name) {
    $app = BossaApp::lookup("short_name='$name'");
    if (!$app) return 0;
    return $app->id;
}

function bossa_lookup_job($instid, &$job, &$inst, &$user) {
    $inst = BossaJobInst::lookup_id($instid);
    if (!$inst) return false;
    $job = BossaJob::lookup_id($inst->job_id);
    $user = BoincUser::lookup_id($inst->user_id);
    BossaUser::lookup($user);
    return true;
}

?>
