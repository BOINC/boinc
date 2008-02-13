<?php

require_once("../inc/bossa_db.inc");

function lookup_bapp($id) {
    global $apps;
    foreach ($apps as $app) {
        if ($app->id == $id) return $app;
    }
    return null;
}

// return the confidence of the instances that agree with the given one
//
function agree_conf($instances, $inst) {
    $sum = $inst->conf;
    foreach ($instances as $i) {
        if ($i->id == $inst->id) continue;
        if (compatible($i, $inst)) {
            $sum += $i->conf;
        }
    }
    return $sum;
}

// return the total confidence
//
function total_confidence($instances) {
    $sum = 0;
    foreach ($instances as $i) {
        if (!$i->finished_time) continue;
        $sum += $i->conf;
    }
    return $sum;
}

// See if there's a canonical instance.
// If there's not, return the confidence of the largest compatible set
//
function find_canonical($instances, $total_conf, &$max_conf) {
    $best = null;
    $best_conf = 0;
    foreach ($instances as $inst) {
        $ac = agree_conf($instances, $inst);
        if ($ac > $best_conf) {
            $best_conf = $ac;
            $best = $inst;
        }
    }
    if (!$best) return;
    $max_conf = $best_conf;
    if ($best_conf < $app->min_conf_sum) return;
    if ($best_conf/$total_conf < $app->min_conf_frac) return;
    return $best;
}

function get_confidences(&$instances) {
    foreach ($instances as $inst) {
        $user = BoincUser::lookup_id($inst->user_id);
        BossaUser::lookup($user);
        $inst->conf = 1;
    }
}

// this gets invoked when
// 1) an instance has been completed
// 2) an instance has timed out
//
function handle_job($job) {
    $app = lookup_bapp($job->app_id);
    if (!$app) {
        echo "Missing app: $job->app_id\n";
        return;
    }
    $instances = BossaJobInst::enum("job_id=$job->id");
    if ($job->canonical_inst_id) {
        // Already have a canonical instance.
        // Are there new instances to validate?
        //
        $canonical_inst = find_canonical($job, $instances);
        foreach ($instances as $inst) {
            switch ($inst->validate_state) {
            case VALIDATE_STATE_INIT:
                if (compatible($inst, $canonical_inst)) {
                    $inst->validate_state = VALIDATE_STATE_VALID;
                } else {
                    $inst->validate_state = VALIDATE_STATE_INVALID;
                }
                $inst->update("validate_state=$inst->validate_state");
            }
        }
    } else {
        // No canonical instance yet.
        // If we have enough total confidence, check for consensus
        //
        get_confidences($instances);
        $total_conf = total_confidence($instances);
        if ($total_conf >= $app->min_conf_sum) {
            $inst = find_canonical($instances, $total_conf, $max_conf);
        }
    }
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
    foreach ($apps as $app) {
        $bs = "../inc/".$app->short_name.".inc";
        require_once($bs);
    }
    while (1) {
        do_pass();
    }
}

main();
echo "foo";
?>
