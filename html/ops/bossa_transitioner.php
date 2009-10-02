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

$cli_only = true;
require_once("../inc/bossa_db.inc");
require_once("../inc/util_ops.inc");

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
