<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
// - N instances are negative
//      and prod(neg_err_rate)<PROB_LIMIT
// - there are 10 finished instances
//   (in which case the job is marked as inconclusive)

require_once("../inc/bossa.inc");

define("PROB_LIMIT", 1e-3);

function job_show($job, $inst, $user) {
    $info = $job->get_opaque_data();
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
    $inst->set_opaque_data($response);

    // if this is a calibration job, update user's opaque data
    //
    if ($job->calibration) {
        $b = $user->bossa;
        $info = $job->get_opaque_data();
        $answer = $info->answer;
        $u = $b->get_opaque_data();
        if (!$u) {
            $u->npos = 0;
            $u->npos_err = 0;
            $u->nneg = 0;
            $u->nneg_err = 0;
        }
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
        $b->set_opaque_data($u);
        return;
    }

    // now see if job is done
    //
    $insts = $job->get_finished_instances();
    $n = count($insts);

    $results = null;
    $users = null;
    foreach ($insts as $inst) {
        $results[] = $inst->get_opaque_data();
        $u = $inst->get_user();
        $users[] = $u->bossa->get_opaque_data();
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
        $job->set_state(BOSSA_JOB_DONE);
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
            $job->set_state(BOSSA_JOB_DONE);
            return;
        }
    }

    // see if there are too many instances without a consensus
    //
    if ($n >= 10) {
        $job->set_state(BOSSA_JOB_INCONCLUSIVE);
        return;
    }

    // still looking for consensus - get another instance
    //
    $job->set_priority(2);

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

function job_summary($job) {
    $info = $job->get_opaque_data();
    return "<a href=".URL_BASE."$info->path>(view image)</a>";
}

function instance_summary($info) {
    if ($info->have_ellipse) {
        return "ellipse ($info->cx, $info->cy)";
    } else {
        return "no ellipse";
    }
}

function user_summary($user) {
    $b = $user->bossa;
    $info = $b->get_opaque_data();
    if ($info) {
        if ($info->npos) {
            $pos_err = $info->npos_err/$info->npos;
        } else {
            $pos_err = "---";
        }
        if ($info->nneg) {
            $neg_err = $info->nneg_err/$info->nneg;
        } else {
            $neg_err = "---";
        }
        return "error rate: positive $pos_err ($info->npos_err/$info->npos),
             negative $neg_err ($info->nneg_err/$info->nneg)
        ";
    } else {
        return "No data";
    }
}

?>
