<?php

require_once("../inc/bossa_db.inc");

function make_jobs() {
    $appname = 'bossa_test';
    $app = BossaApp::lookup_name($appname);
    if (!$app) {
        echo "Application $appname not found\n";
        exit(1);
    }
    $job = new BossaJob;
    $job->app_id = $app->id;
    $job->batch = 0;
    $job->time_estimate = 30;
    $job->time_limit = 600;
    $job->nsuccess_needed = 3;
    for ($i=0; $i<10; $i++) {
        $job->name = "job_$i";
        $info = null;
        $info->number = $i % 2;
        $job->info = json_encode($info);
        if (!$job->insert()) {
            echo "BossaJob::insert failed: ", mysql_error(), "\n";
            exit(1);
        }
    }
}

make_jobs();
echo "All done.\n";

?>
