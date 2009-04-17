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

require_once("../inc/bossa_db.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/util_ops.inc");

function job_state_string($s) {
    switch ($s) {
    case 0: return "Embargoed";
    case 1: return "In progress";
    case 2: return "Completed";
    }
}

function include_app_file($app_id) {
    $app = BossaApp::lookup_id($app_id);
    $file = "../inc/$app->short_name.inc";
    require_once($file);
}

function show_bossa_app($app, $i) {
    $j = $i%2;
    echo "<tr class=row$j>
        <td>Name: $app->name<br>
            Short name: $app->short_name<br>
            Description: $app->description<br>
            Created: ".date_str($app->create_time)."
        </td>
        <td>
    ";
    if ($app->hidden) {
        show_button("bossa_admin.php?action=unhide&app_id=$app->id", "Unhide", "Unhide this app");
    } else {
        show_button("bossa_admin.php?action=hide&app_id=$app->id", "Hide", "Hide this app");
    }
    echo "<br>";
    show_button("bossa_admin.php?action=show_batches&app_id=$app->id", "Show batches", "Show batches");
}

function show_bossa_apps() {
    $apps = BossaApp::enum();
    start_table();
    row1("Existing apps", 2);
    table_header("Name/description", "");
    $i = 0;
    foreach ($apps as $app) {
        show_bossa_app($app, $i++);
    }
    end_table();
}

function add_app_form() {
    echo "
        <form action=bossa_admin.php method=get>
        <input type=hidden name=action value=add_app>
    ";
    start_table();
    row1("Add app");
    row2(
        "Name<span class=note><br>Visible to users</span>",
        "<input name=app_name>"
    );
    row2(
        "Short name<span class=note><br>Used in file and function names - no spaces or special characters</span>",
        "<input name=short_name>"
    );
    row2(
        "Description<span class=note><br>Visible to users</span>",
        "<textarea name=description cols=60></textarea>"
    );
    row2("Average time per job", "<input name=time_estimate> seconds");
    row2("Time limit per job", "<input name=time_limit> seconds");
    row2("Fraction of calibration jobs", "<input name=calibration_frac>");
    row2("Name of Bolt training course", "<input name=training_course>");
    row2("", "<input type=submit submit value=\"Create app\">");
    end_table();
    echo "</form>";
}

function user_settings() {
    global $user;
    $flags = $user->bossa->flags;
    echo "<form action=bossa_admin.php method=get>
        <input type=hidden name=action value=update_user>
    ";
    start_table();
    row1("User settings");
    $x = ($flags&BOLT_FLAGS_SHOW_ALL)?"checked":"";
    row2("Show hidden apps?", "<input type=checkbox name=show_all $x>");
    $x = ($flags&BOLT_FLAGS_DEBUG)?"checked":"";
    row2("Show debugging output?", "<input type=checkbox name=debug $x>");
    row2("", "<input type=submit value=\"Update user\">");
    end_table();
    echo "</form>";
}

function show_all() {
    admin_page_head("Bossa administration");
    show_bossa_apps();
    echo "<p>";
    add_app_form();
    echo "<p>";
    user_settings();
    admin_page_tail();
}

function job_duration($inst) {
    if ($inst->finish_time) {
        $d = $inst->finish_time - $inst->create_time;
        $d /= 60;
        $d = number_format($d, 2);
        $d = "$d min.";
    } else {
        $d = "---";
    }
    return $d;
}

function job_show_instances($job) {
    $insts = BossaJobInst::enum("job_id=$job->id");
    if (!count($insts)) {
        echo "---";
        return;
    }
    start_table();
    table_header("User", "Start", "Duration", "Result");
    foreach ($insts as $inst) {
        $user = BoincUser::lookup_id($inst->user_id);
        $t = time_str($inst->create_time);
        $d = job_duration($inst);
        echo "<tr>
            <td><a href=bossa_admin.php?action=show_user&app_id=$job->app_id&user_id=$user->id>$user->name</a></td>
            <td>$t</td>
            <td>$d</td>
            <td>
        ";
        echo instance_summary($inst->get_opaque_data());
        echo "
            </td>
            </tr>
        ";
    }
    end_table();
}

function show_batch($batch_id) {
    $batch = BossaBatch::lookup_id($batch_id);
    if (!$batch) error_page("No such batch");
    include_app_file($batch->app_id);
    admin_page_head("Jobs for batch $batch->name");
    $jobs = BossaJob::enum("batch_id=$batch_id");
    start_table();
    table_header("ID", "Created", "State", "Instances");
    $i = 0;
    foreach ($jobs as $job) {
        $t = time_str($job->create_time);
        $s = job_state_string($job->state);
        $j = $i++ % 2;
        echo "<tr class=row$j>
            <td>
                $job->id <a href=bossa_admin.php?action=job_show_insts&job_id=$job->id>(details)</a><br>
        ";
        echo job_summary($job);
        echo "
            </td>
            <td>$t</td>
            <td>$s</td>
            <td>
        ";
        job_show_instances($job);
        echo "
            </td>
            </tr>
        ";
    }
    end_table();
    admin_page_tail();
}

function show_batches($app_id) {
    $batches = BossaBatch::enum("app_id = $app_id");
    $app = BossaApp::lookup_id($app_id);
    admin_page_head("Batches ($app->name)");
    start_table();
    table_header("ID", "Name", "Calibration?", "Created", "Jobs", "Completed");
    foreach ($batches as $batch) {
        table_row(
            "$batch->id | <a href=bossa_admin.php?action=show_batch&batch_id=$batch->id>show jobs</a> | <a href=bossa_admin.php?action=clear_batch_confirm&batch_id=$batch->id>delete instances</a>",
            "$batch->name",
            $batch->calibration?"yes":"no",
            time_str($batch->create_time),
            BossaJob::count("batch_id=$batch->id"),
            BossaJob::count("batch_id=$batch->id and state=2")
        );
    }
    end_table();
    admin_page_tail();
}

function job_show_insts($job_id) {
    $job = BossaJob::lookup_id($job_id);
    include_app_file($job->app_id);
    admin_page_head("Instances of job $job_id");
    job_show_instances($job);
    admin_page_tail();
}

function calibration_job_string($inst, $job) {
    if ($inst->calibration) {
        $i = $job->get_opaque_data();
        return "yes: ".instance_summary($i->answer);
    } else {
        return "no";
    }
}

function show_bossa_user() {
    $user_id = get_int('user_id');
    $app_id = get_int('app_id');
    $user = BoincUser::lookup_id("$user_id");
    BossaUser::lookup($user);
    $app = BossaApp::lookup_id($app_id);

    include_app_file($app_id);
    admin_page_head("$user->name ($app->name)");
    $x = user_summary($user);
    if ($x) {
        echo "User info: $x<br>";
    }
    $insts = BossaJobInst::enum("user_id=$user_id");
    start_table();
    table_header("Job", "Calibration?", "Start", "Duration", "Response");
    foreach ($insts as $inst) {
        $job = BossaJob::lookup_id($inst->job_id);
        table_row(
            "$inst->job_id <a href=bossa_admin.php?action=job_show_insts&job_id=$inst->job_id>(details)</a><br>".job_summary($job),
            calibration_job_string($inst, $job),
            time_str($inst->create_time),
            job_duration($inst),
            instance_summary($inst->get_opaque_data())
        );
    }
    end_table();
    admin_page_tail();
}

function clear_batch_confirm($batch_id) {
    admin_page_head("Confirm delete instances");
    echo "
        This will permanently delete all results from this batch.
        <p>
        Are you sure you want to do this?
        <p>
        <a href=bossa_admin.php?action=clear_batch&batch_id=$batch_id>Yes</a>
    ";
    admin_page_tail();
}

function clear_batch($batch_id) {
    admin_page_head("Deleting instances");
    if (BossaJobInst::delete_aux("batch_id=$batch_id")) {
        echo "Job instances deleted.";
        $jobs = BossaJob::enum("batch_id=$batch_id");
        foreach ($jobs as $job) {
            $job->update("priority_0=1");
        }
    } else {
        echo "Database error.";
    }
    admin_page_tail();
}

$user = get_logged_in_user();

$db = BossaDb::get();
if (!$db) error_page("Can't connect to database server");

if (!$db->table_exists('bossa_app')) {
    admin_page_head("Create Bossa database");
    $db_name = $db->db_name;
    echo "
        The database tables for Bossa don't seem to exist.
        To create them, go to ~/boinc/db and type
        <pre>
mysql $db_name < bossa_schema.sql
mysql $db_name < bossa_constraints.sql
</pre>
    Then <a href=bossa_admin.php>reload this page</a>.
    ";
    admin_page_tail();
    exit;
}

BossaUser::lookup($user);

$action = get_str('action', true);
switch ($action) {
case 'add_app':
    $name = BossaDb::escape_string(get_str('app_name'));
    $short_name = get_str('short_name');
    $description = BossaDb::escape_string(get_str('description'));
    $training_course = get_str('training_course', true);
    if (strlen($training_course)) {
        $course = BoltCourse::lookup_name($training_course);
        if (!$course) {
            error_page("No course named $training_course");
        }
        $courseid = $course->id;
    } else {
        $courseid = 0;
    }
    $time_estimate = get_str('time_estimate');
    if (!$time_estimate) $time_estimate = 60;
    $time_limit = get_str('time_limit');
    if (!$time_limit) $time_limit = 3600;
    $calibration_frac = get_str('calibration_frac' , true);
    if (!$calibration_frac) $calibration_frac = 0;
    $now = time();
    $app_id = BossaApp::insert("(create_time, name, short_name, description, bolt_course_id, time_estimate, time_limit, calibration_frac) values ($now, '$name', '$short_name', '$description', $courseid, $time_estimate, $time_limit, $calibration_frac)");
    if ($courseid) {
        $course->update("bossa_app_id=$app_id");
    }
    Header('Location: bossa_admin.php');
    exit;
case 'update_user':
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bossa->update("flags=$flags");
    $user->bossa->flags = $flags;
    Header('Location: bossa_admin.php');
    exit;
case 'show_user':
    show_bossa_user();
    exit;
case 'show_batches':
    $app_id = get_int('app_id');
    show_batches($app_id);
    exit;
case 'show_batch':
    $batch_id = get_int('batch_id');
    show_batch($batch_id);
    exit;
case 'job_show_insts':
    $job_id = get_int('job_id');
    job_show_insts($job_id);
    exit;
case 'hide':
    $app_id = get_int('app_id');
    $app = BossaApp::lookup_id($app_id);
    if (!$app) error_page("no such app");
    $app->update("hidden=1");
    break;
case 'unhide':
    $app_id = get_int('app_id');
    $app = BossaApp::lookup_id($app_id);
    if (!$app) error_page("no such app");
    $app->update("hidden=0");
    break;
case 'clear_batch_confirm':
    $batch_id = get_int('batch_id');
    clear_batch_confirm($batch_id);
    exit;
case 'clear_batch':
    $batch_id = get_int('batch_id');
    clear_batch($batch_id);
    exit;
case '':
    show_all();
    exit;
default:
    error_page("unknown action $action");
}
Header('Location: bossa_admin.php');


?>
