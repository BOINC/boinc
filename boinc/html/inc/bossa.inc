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

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

function bossa_job_create(
    $app_id, $batch_id, $info, $calibration
) {
    $job = new BossaJob;
    $info = serialize($info);
    if ($calibration) {
        $priority = drand();
        $c = 1;
    } else {
        $priority = 1;
        $c = 0;
    }

    $now = time();
    $int_max = 2147483647;
    $clause = "(create_time, app_id, batch_id, state, info, calibration, priority_0) values ($now, $app_id, $batch_id, 1, '$info', $c, $priority)";
    return $job->insert($clause);
}

function bossa_batch_create($appid, $name, $calibration) {
    $now = time();
    $c = $calibration?"1":"0";
    return BossaBatch::insert("(create_time, app_id, name, calibration) values ($now, $appid, '$name', $c)");
}

function bossa_app_lookup($name) {
    $app = BossaApp::lookup("short_name='$name'");
    if (!$app) return 0;
    return $app->id;
}

function bossa_lookup_job($instid, &$job, &$inst, &$user) {
    $inst = BossaJobInst::lookup_id($instid);
    if (!$inst) return false;
    $job = BossaJob::lookup_id($inst->job_id);
    $user = BoincUser::lookup_id($inst->user_id);
    BossaUser::lookup($user);
    return true;
}

?>
