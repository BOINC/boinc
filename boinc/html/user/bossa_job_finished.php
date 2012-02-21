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
require_once("../inc/bossa_impl.inc");

$user = get_logged_in_user();
$inst = BossaJobInst::lookup_id(get_int('bji'));
if (!$inst) {
    error_page("No such job instance");
}
if ($inst->user_id != $user->id) {
    error_page("Bad user ID");
}
if ($inst->finish_time) {
    error_page("You already finished this job");
}
$job = BossaJob::lookup_id($inst->job_id);
if (!$job) {
    error_page("No such job");
}

$app = BossaApp::lookup_id($job->app_id);
$file = "../inc/$app->short_name.inc";
require_once($file);

{
    $trans = new BossaTransaction();

    $now = time();
    $inst->update("finish_time=$now, timeout=0");

    BossaUser::lookup($user);
    job_finished($job, $inst, $user);
    show_next_job($app, $user);
}

?>
