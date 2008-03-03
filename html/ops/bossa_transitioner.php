<?php

require_once("../inc/bossa_db.inc");

$int_max = 2147483647;
$now = 0;

function debug($x) {
    //return;
    echo "$x\n";
}

function lookup_bapp($id) {
    global $apps;
    foreach ($apps as $app) {
        if ($app->id == $id) return $app;
    }
    return null;
}

// is the job instance timed out?
//
function timed_out($ji, $job) {
    $deadline = $ji->create_time + $job->time_limit;
}

// return the total confidence of the instances that agree with the given one
//
function agree_conf($app, $instances, $inst) {
    $sum = $inst->confidence;
    foreach ($instances as $i) {
        if (!$i->finish_time) continue;
        if ($i->id == $inst->id) continue;
        $c = $app->compare;
        if ($c($i, $inst)) {
            $sum += $i->confidence;
        }
    }
    return $sum;
}

// return the total confidence of the finished instances
//
function finished_confidence($instances) {
    $sum = 0;
    foreach ($instances as $i) {
        if (!$i->finish_time) continue;
        $sum += $i->confidence;
    }
    return $sum;
}

// return the total confidence of the viable instances
//
function viable_confidence($instances, $job) {
    $sum = 0;
    foreach ($instances as $i) {
        if (!$i->finish_time) continue;
        if (timed_out($i, $job)) continue;
        $sum += $i->confidence;
    }
    return $sum;
}

// See if there's a canonical instance.
// If there's not, return the confidence of the largest compatible set
//
function find_canonical($app, $instances, $finished_conf, &$max_conf) {
    $best = null;
    $best_conf = 0;
    foreach ($instances as $inst) {
        if (!$inst->finish_time) continue;
        $ac = agree_conf($app, $instances, $inst);
        debug("  agree conf for $inst->id: $ac");
        if ($ac > $best_conf) {
            $best_conf = $ac;
            $best = $inst;
        }
    }
    if (!$best) return;
    $max_conf = $best_conf;
    if ($best_conf < $app->min_conf_sum) return;
    if ($best_conf/$finished_conf < $app->min_conf_frac) return;
    return $best;
}

// A canonical instance has just been identified
// Mark instances as valid/invalid, and call the app's handler function
//
function handle_canonical($app, $job, $cinst, $instances) {
    $valid_instances = array();
    $c = $app->compare;
    foreach ($instances as $inst) {
        if (!$inst->finish_time) continue;
        if ($inst->id == $cinst->id) {
            $inst->validate_state = VALIDATE_STATE_VALID;
            $valid_instances[] = $inst;
        } else if ($c($inst, $cinst)) {
            $inst->validate_state = VALIDATE_STATE_VALID;
            $valid_instances[] = $inst;
        } else {
            $inst->validate_state = VALIDATE_STATE_INVALID;
        }
        $inst->update("validate_state=$inst->validate_state");
    }
    $h = $app->handle;
    $h($job, $valid_instances);
    $job->update("canonical_inst_id=$cinst->id");
}

function lookup_canonical($instances, $id) {
    foreach ($instances as $i) {
        if ($i->id == $id) return $i;
    }
    return null;
}

// this gets invoked when
// 1) an instance has been completed
// 2) an instance has timed out
//
function handle_job($job) {
    global $int_max;
    global $now;

    $next_transition = $int_max;
    debug("processing job $job->id ($job->name)");
    $app = lookup_bapp($job->app_id);
    if (!$app) {
        echo "ERROR: missing app: $job->app_id\n";
        $job->update("transition_time=$int_max");
        return;
    }
    $instances = BossaJobInst::enum("job_id=$job->id");
    if ($job->canonical_inst_id) {
        // Already have a canonical instance.
        // Are there new instances to validate?
        //
        debug("  Already have CI $job->canonical_inst_id");
        $canonical_inst = lookup_canonical($instances, $job->canonical_inst_id);
        if (!$canonical_inst) {
            echo "ERROR: can't find canonical instance for job $job->id\n";
            return;
        }
        foreach ($instances as $inst) {
            switch ($inst->validate_state) {
            case VALIDATE_STATE_INIT:
                if (compatible($inst, $canonical_inst)) {
                    $inst->validate_state = VALIDATE_STATE_VALID;
                } else {
                    $inst->validate_state = VALIDATE_STATE_INVALID;
                }
                debug("  Instance $inst->id: valid=$inst->validate_state");
                $inst->update("validate_state=$inst->validate_state");
            }
        }
        $conf_needed = 0;
    } else {
        // No canonical instance yet.
        // If we have enough total confidence, check for consensus
        //
        $finished_conf = finished_confidence($instances);
        $vc = viable_confidence($instances, $job);
        debug("  Don't have CI yet; finished confidence is $finished_conf");
        if ($finished_conf >= $app->min_conf_sum) {
            $inst = find_canonical($app, $instances, $finished_conf, $max_conf);
            if ($inst) {
                debug("  Found CI ($inst->id); max conf $max_conf finished conf $finished_conf");
                handle_canonical($app, $job, $inst, $instances);
                $conf_needed = 0;
            } else {
                debug("  No CI found; max conf $max_conf finished conf $finished_conf");
                $f = $app->min_conf_frac;
                $conf_needed = ($f*$vc - $max_conf)/(1-$f);
                debug("  f $f vc $vc max_conf $max_conf -> conf_needed $conf_needed");
            }
        } else {
            $conf_needed = $app->min_conf_sum - $vc;
        }
        foreach ($instances as $inst) {
            if (!$inst->finish_time) {
                $deadline = $inst->create_time + $job->time_limit;
                if ($deadline > $now) {
                    if ($deadline < $next_transition) $next_transition = $deadline;
                }
            }
        }
    }

    // set job's new transition time and conf_needed
    //
    debug("  New transition time $next_transition, conf_needed $conf_needed");
    $job->update("transition_time=$next_transition, conf_needed=$conf_needed");
}

function do_pass() {
    global $now;
    $now = time();
    $jobs = BossaJob::enum("transition_time < $now");
    if (!count($jobs)) return false;
    foreach ($jobs as $job) {
        handle_job($job);
    }
    return true;
}

function main() {
    global $apps;
    $apps = BossaApp::enum();
    foreach ($apps as $app) {
        $bs = "../inc/".$app->short_name.".inc";
        require_once($bs);
        $app->compare = $app->short_name."_compare";
        $app->handle = $app->short_name."_handle";
    }
    while (1) {
        if (!do_pass()) {
            debug("Sleeping");
            sleep(10);
        }
    }
}

main();
?>
