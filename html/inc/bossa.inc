<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

function bossa_job_create(
    $app_id, $batch_id, $info, $is_calibration, $priority
) {
    $job = new BossaJob;
    $info = serialize($info);
    $is_calibration = $is_calibration?1:0;

    $now = time();
    $int_max = 2147483647;
    $clause = "(create_time, app_id, batch_id, state, info, is_calibration, priority_0) values ($now, $app_id, $batch_id, 1, '$info', $is_calibration, $priority)";
    return $job->insert($clause);
}

function bossa_batch_create($appid, $name) {
    $now = time();
    return BossaBatch::insert("(create_time, app_id, name) values ($now, $appid, '$name')");
}

function bossa_app_lookup($name) {
    $app = BossaApp::lookup("short_name='$name'");
    if (!$app) return 0;
    return $app->id;
}

?>
