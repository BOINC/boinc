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
require_once("../inc/bolt_db.inc");

function show_app($app) {
    global $user;
    if ($app->bolt_course_id) {
        if ($user) {
            switch (bolt_course_status($app->bolt_course_id, $user->id)) {
            case BOLT_COURSE_NOT_STARTED:
                $x = "<a href=bolt_sched.php?action=start&course_id=$app->bolt_course_id>Take training course</a>";
                break;
            case BOLT_COURSE_STARTED:
                $x = "<a href=bolt_sched.php?action=resume&course_id=$app->bolt_course_id>Finish training course</a>";
                break;
            case BOLT_COURSE_FINISHED:
                $x = "<a href=bossa_get_job.php?bossa_app_id=$app->id>Get job</a>";
                break;
            }
        } else {
            $x = "<a href=bolt_sched.php?action=start&course_id=$app->bolt_course_id>Take training course</a>";
        }
    } else {
        $x = "<a href=bossa_get_job.php?bossa_app_id=$app->id>Get job</a>";
    }
    $est = number_format($app->time_estimate/60., 2);
    $limit = number_format($app->time_limit/60., 2);
    row2("$app->name<br><span class=note>$app->description<br>Time: $est min. average, $limit min limit</span>", $x);
}

function show_apps() {
    $apps = BossaApp::enum();
    foreach ($apps as $app) {
        if ($app->hidden) continue;
        show_app($app);
    }
}

function main() {
    page_head("Bossa apps");
    start_table();
    show_apps();
    end_table();
    page_tail();
}

$user = get_logged_in_user();

main();

?>
