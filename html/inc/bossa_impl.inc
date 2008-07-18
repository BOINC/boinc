<?php

function show_next_job($app, $user) {
    $inst = BossaJobInst::assign($app, $user);
    if ($inst) {
        $job = BossaJob::lookup_id($inst->job_id);
        $file = "../inc/$app->short_name.inc";
        require_once($file);
        job_issued($job, $inst, $user);
        page_head("Bossa job");
        job_show($job, $inst, $user);
        page_tail();
    } else {
        page_head("No jobs available");
        echo "
            Sorry, no jobs are available right now.
            <p>
            Please try again later.
        ";
        page_tail();
    }
}

?>
