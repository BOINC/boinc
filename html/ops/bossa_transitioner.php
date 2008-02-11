<?php

require_once("../inc/bossa_db.inc");

var $apps;

function lookup_app($id) {
    global $apps;
    foreach ($apps as $app) {
        if ($app->id == $id) return $app;
    }
    return null;
}

function handle_job($job) {
    $app = lookup_app($job->app_id);
    if (!$app) {
        echo "Missing app: $job->app_id\n";
        return;
    }
    $instances = BossaJobInst::enum("job_id=$job->id");
}

function do_pass() {
    $now = time();
    $jobs = BossaJob::enum("transition_time < $now");
    foreach ($jobs as $job) {
        handle_job($job);
    }
}

function main() {
    global $apps;
    $apps = BossaApp::enum();
    while (1) {
        do_pass();
    }
}

main();
?>
