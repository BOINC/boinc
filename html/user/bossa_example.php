<?php

require_once("../inc/bossa.inc");

// Bossa example.
// Show the user an image and ask them to click on the ellipse

function job_show($job, $inst, $user) {
    $info = bossa_job_get_info($job);
    $path = $info->path;
    echo "
        <h2>Find the Ellipse!</h2>
        <form method=get action=bossa_job_finished.php>
        Click on the center of the ellipse.
        If you don't see one, click here:
        <input type=submit name=submit value=None>
        <br><br>
        <input type=hidden name=bji value=$inst->id>
        <input type=image name=pic src=$img_url>
        </form>
    ";
}

function job_issued($job, $inst, $user) {
    bossa_job_set_priority($job, 0);
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
    bossa_update_job_info($inst, $response);
}

function job_timed_out($job, $inst, $user) {
    bossa_job_set_priority($job, 1);
}

?>
