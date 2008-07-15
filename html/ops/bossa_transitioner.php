<?php

require_once("../inc/bossa_db.inc");

function do_pass() {
    $int_max = 2147483647;
    $now = time();
    $insts = BossaJobInst::enum("timeout < $now");
    if (!count($insts)) return false;
    foreach ($insts as $inst) {
        BossaDb::start_transaction();
        $inst = BossaJobInst::lookup_id($inst->id);
            // reread instance within transation
        if ($inst->transition_time < $now) {
            $job = BossaJob::lookup_id($inst->job_id);
            $user = BoincUser::lookup_id($inst->user_id);
            BossaUser::lookup($user);
            job_timed_out($job, $inst, $user);
        }
        $inst->update("timeout=$int_max");
        BossaDb::commit();
    }
    return true;
}

$app_name = $argv[1];
$app = BossaApp::lookup("short_name='$app_name'");
if (!$app) {
    echo "No app named $app_name\n";
    exit;
}

$bs = "../inc/".$app_name.".inc";
require_once($bs);
while (1) {
    if (!do_pass()) {
        echo("Sleeping\n");
        sleep(10);
    }
}

?>
