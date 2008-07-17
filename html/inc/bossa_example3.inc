<?php

// Bossa example #3 - calibration jobs.
// We maintain the following for each user:
// nneg         # of negative calibration jobs shown
// nneg_err     # of errors
// npos
// npos_err
//
// from these we derive
// neg_err_rate
// pos_err_rate
//
// a job is considered done if either
// - N instances are positive and match within +- 20 pixels,
//      and prod(pos_err_rate)<PROB_LIMIT
// - 2 instances are negative
//      and prod(neg_err_rate)<PROB_LIMIT
// - there are 10 finished instances
//   (in which case the job is marked as inconclusive)

require_once("../inc/bossa.inc");

define("PROB_LIMIT", 1e-3);

function job_show($job, $inst, $user) {
    $info = $job->get_info($job);
    $path = $info->path;
    echo "JOB SHOW";
    echo "
        <h2>Find the Ellipse!</h2>
        <form method=get action=bossa_job_finished.php>
        Click on the center of the ellipse.
        If you don't see one, click here:
        <input type=submit name=submit value=None>
        <br><br>
        <input type=hidden name=bji value=$inst->id>
        <input type=image name=pic src=$path>
        </form>
    ";
}

function job_issued($job, $inst, $user) {
    echo "JOB ISSUED";
    $insts = $job->get_instances();
    if (count($insts) > 1) {
        $job->set_priority(0);
    }
}

function job_finished($job, $inst, $user) {
    $response = null;
    if (get_str('submit', true)) {
        $response->have_ellipse = 0;
    } else {
        $response->have_ellipse = 1;
        $response->cx = get_int('pic_x');
        $response->cy = get_int('pic_y');
    }
    $inst->update_info($response);

    if ($job->calibration) {
        $info = $job->get_info();
        $answer = $info->answer;
        if (compatible($response, $answer)) {
            if ($answer->have_ellipse) {
                $u->npos++;
            } else {
                $u->nneg++;
            }
        } else {
            if ($answer->have_ellipse) {
                $u->npos++;
                $u->npos_err++;
            } else {
                $u->nneg++;
                $u->nneg_err++;
            }
        }
        $b = $user->bossa;
        $b->update_info($info);
        return;
    }

    // see if job is done
    //
    $insts = $job->get_finished_instances();
    $n = count($insts);

    $results = null;
    $users = null;
    foreach ($insts as $inst) {
        $results[] = $inst->get_info();
        $u = $inst->get_user();
        $users[] = $u->bossa->get_info();
    }

    // see if there's a negative consensus
    //
    $prob = 1;
    for ($i=0; $i<$n; $i++) {
        $r = $results[$i];
        if ($r1->have_ellipse) continue;
        $u = $users[$i];
        $prob *= $u->neg_err_rate;
    }
    if ($prob < PROB_LIMIT) {
        $job->update_state(BOSSA_JOB_DONE);
        return;
    }

    // see if there's a positive consensus
    //
    for ($i=0; $i<$n; $i++) {
        $r1 = $results[$i];
        $u = $users[$i];
        $prob = $u->pos_error_rate;
        for ($j=0; $j<$n; $j++) {
            if ($j == $i) continue;
            $r2 = $results[$j];
            if (compatible($r1, $r2)) {
                $u2 = $users[$j];
                $prob *= $u2->pos_err_rate;
            }
        }
        if ($prob < PROB_LIMIT) {
            $job->update_state(BOSSA_JOB_DONE);
            return;
        }
    }

    // see if there are too many instances without a consensus
    //
    if ($n >= 10) {
        $job->update_state(BOSSA_JOB_INCONCLUSIVE);
        return;
    }
}

// two results are compatible if neither found an ellipse,
// or they both did and centers are within 20 pixels
//
function compatible($r1, $r2) {
    if ($r1->have_ellipse) {
        if ($r2->have_ellipse) {
            $dx = ($r1->cx - $r2->cx);
            $dy = ($r1->cy - $r2->cy);
            $dsq = $dx*$dx + $dy*$dy;
            return ($dsq < 400);
        } else return false;
    } else {
        return !$r2->have_ellipse;
    }
}

function job_timed_out($job, $inst, $user) {
    $job->set_priority(1);
}

function show_job_summary($job) {
    $info = $job->get_info();
    echo "<a href=".URL_BASE."$info->path>View image</a>";
}

function show_instance_summary($inst) {
    $info = $inst->get_info();
    if ($info->have_ellipse) {
        echo "($info->cx, $info->cy)";
    } else {
        echo "---";
    }
}

function show_user_summary($user) {
}

?>
