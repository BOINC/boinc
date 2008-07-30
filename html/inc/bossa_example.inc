<?php

require_once("../inc/bossa.inc");

// Bossa example #1.
// Show the user an image and ask them to click on the ellipse
// This version does no replication.

function job_show($job, $inst, $user) {
    $info = $job->get_opaque_data($job);
    $path = $info->path;
    page_head("Find the Ellipse");
    echo "
        <form method=get action=bossa_job_finished.php>
        Click on the center of the ellipse.
        If you don't see one, click here:
        <input type=submit name=submit value=None>
        <br><br>
        <input type=hidden name=bji value=$inst->id>
        <input type=image name=pic src=$path>
        </form>
    ";
    page_tail();
}

function job_issued($job, $inst, $user) {
    $job->set_priority(0);
}

function job_finished($job, $inst) {
    $response = null;
    if (get_str('submit', true)) {
        $response->have_ellipse = 0;
    } else {
        $response->have_ellipse = 1;
        $response->cx = get_int('pic_x');
        $response->cy = get_int('pic_y');
    }
    $inst->set_opaque_data($response);
}

function job_timed_out($job, $inst, $user) {
    $job->set_priority(1);
}

function job_summary($job) {
    $info = $job->get_opaque_data();
    return "<a href=".URL_BASE."$info->path>View image</a>";
}

function instance_summary($inst) {
    $info = $inst->get_opaque_data();
    if ($info->have_ellipse) {
        return "($info->cx, $info->cy)";
    } else {
        return "no ellipse";
    }
}

function show_user_summary($user) {
}

?>
