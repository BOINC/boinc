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

function show_next_job($app, $user) {
    $inst = BossaJobInst::assign($app, $user);
    if ($inst) {
        $job = BossaJob::lookup_id($inst->job_id);
        $file = "../inc/$app->short_name.inc";
        require_once($file);
        job_issued($job, $inst, $user);
        job_show($job, $inst, $user);
    } else {
        page_head("No jobs available");
        echo "
            Sorry, no jobs are available right now.
            <p>
            Please try again later.
        ";
    }
    page_tail();
}

?>
