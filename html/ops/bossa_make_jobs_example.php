<?php

require_once("../bossa_inc/bossa_db.inc");
require_once("../inc/db.inc");

db_init();

function make_jobs() {
    $app = Bossa::app_lookup_name('bossa_test');
    if (!$app) {
        echo "No app\n";
        exit(1);
    }
    $job->app_id = $app->id;
    $job->batch = 0;
    $job->time_estimate = 30;
    $job->time_limit = 600;
    $job->nsuccess_needed = 3;
    for ($i=0; $i<10; $i++) {
        $job->name = "job_$i";
        $job->job_info = "$i";
        if (!Bossa::insert_job($job)) {
            echo "failed: ", mysql_error();
            exit(1);
        }
    }
}

make_jobs();

?>
