<?php

require_once("../inc/bossa_db.inc");

$int_max = 2147483647;

function debug($x) {
    //return;
    echo "$x\n";
}

function do_pass() {
    global $int_max;
    $now = time();
    $insts = BossaJobInst::enum("transition_time < $now");
    if (!count($insts)) return false;
    foreach ($insts as $inst) {
        BossaDb::start_transaction();
        $inst = BossaJobInst::lookup_id($inst->id);
            // reread instance within transation
        if ($inst->transition_time < $now) {
            $job = BossaJob::lookup_id($inst->job_id);
            $user = BoincUser::lookup_id($inst->user_id);
            BossaUser::lookup($user);
            if ($inst->finished_time) {
                job_finished($job, $inst, $user);
            } else {
                job_timed_out($job, $inst, $user);
            }
        }
        $inst->update("transition_time=$int_max");
        BossaDb::commit();
    }
    return true;
}

$name = $argv[1];
$app = BossaApp::lookup("short_name='$name'");
if (!$app) {
    echo "No app named $name\n";
    exit;
}

$bs = "../inc/".$name.".inc";
require_once($bs);
while (1) {
    if (!do_pass()) {
        debug("Sleeping");
        sleep(10);
    }
}

?>
