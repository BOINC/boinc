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

require_once("../inc/bossa.inc");

// Bossa example #2.
// Show the user an image and ask them to click on the ellipse
// This version does replication;
// a job is considered done if either
// - 2 instances are positive and match within +- 20 pixels
// - 2 instances are negative
// - there are 10 finished instances
//   (in which case the job is marked as inconclusive)

function job_show($job, $inst, $user) {
    $info = $job->get_opaque_data($job);
    $path = $info->path;
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
    $insts = $job->get_instances();
    if (count($insts) == 1) {
        $job->set_priority(2);
    } else {
        $job->set_priority(0);
    }
}

function job_finished($job, $inst) {
    // record the user's response
    //
    $response = null;
    if (get_str('submit', true)) {
        $response->have_ellipse = 0;
    } else {
        $response->have_ellipse = 1;
        $response->cx = get_int('pic_x');
        $response->cy = get_int('pic_y');
    }
    $inst->set_opaque_data($response);

    // see whether we have a consensus
    //
    $insts = $job->get_finished_instances();
    $n = count($insts);

    $results = null;
    foreach ($insts as $inst) {
        $results[] = $inst->get_opaque_data();
    }
    for ($i=0; $i<$n-1; $i++) {
        $r1 = $results[$i];
        for ($j=$i+1; $j<$n; $j++) {
            $r2 = $results[$j];
            if (compatible($r1, $r2)) {
                $job->set_state(BOSSA_JOB_DONE);
                return;
            }
        }
    }

    // no consensus - see whether we've reached replication limit
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
    $job->set_priority(2);
}

function job_summary($job) {
    $info = $job->get_opaque_data();
    return "<a href=".URL_BASE."$info->path>View image</a>";
}

function instance_summary($info) {
    if ($info->have_ellipse) {
        return "ellipse ($info->cx, $info->cy)";
    } else {
        return "no ellipse";
    }
}

function user_summary($user) {
    return "";
}
