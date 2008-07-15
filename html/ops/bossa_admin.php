<?php

require_once("../inc/bossa_db.inc");
require_once("../inc/bolt_db.inc");
require_once("../inc/util.inc");

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

function show_app($app) {
    echo "<tr>
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

function show_apps() {
    $apps = BossaApp::enum();
    start_table();
    row1("Existing apps", 2);
    table_header("Name/description", "");
    foreach ($apps as $app) {
        show_app($app);
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
    page_head("Bossa administration");
    show_apps();
    echo "<p>";
    add_app_form();
    echo "<p>";
    user_settings();
    page_tail();
}

function show_instances($job_id) {
    $insts = BossaJobInst::enum("job_id=$job_id");
    if (!count($insts)) {
        echo "---";
        return;
    }
    start_table();
    table_header("User", "Start", "Duration", "Result");
    foreach ($insts as $inst) {
        $user = BoincUser::lookup_id($inst->user_id);
        $t = time_str($inst->create_time);
        if ($inst->finish_time) {
            $d = $inst->finish_time - $inst->create_time;
            $d /= 60;
            $d = number_format($d, 2);
            $d = "$d min.";
        } else {
            $d = "---";
        };
        echo "<tr>
            <td><a href=show_user.php?userid=$user->id>$user->name</a></td>
            <td>$t</td>
            <td>$d</td>
            <td>
        ";
        show_instance_summary($inst);
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
    page_head("Jobs for batch $batch->name");
    $jobs = BossaJob::enum("batch_id=$batch_id");
    start_table();
    table_header("ID", "Created", "State", "Instances");
    foreach ($jobs as $job) {
        $t = time_str($job->create_time);
        $s = job_state_string($job->state);
        echo "<tr>
            <td>
                <a href=bossa_admin.php?action=show_insts&job_id=$job->id>$job->id</a><br>
        ";
        show_job_summary($job);
        echo "
            </td>
            <td>$t</td>
            <td>$s</td>
            <td>
        ";
        show_instances($job->id);
        echo "
            </td>
            </tr>
        ";
    }
    end_table();
    page_tail();
}

function show_batches($app_id) {
    $batches = BossaBatch::enum("app_id = $app_id");
    page_head("Batches");
    start_table();
    table_header("ID", "Name", "Calibration?", "Created", "Jobs", "Completed");
    foreach ($batches as $batch) {
        $n = BossaJob::count("batch_id=$batch->id");
        $c = BossaJob::count("batch_id=$batch->id and state=2");
        $t = time_str($batch->create_time);
        $cal = $batch->calibration?"yes":"no";
        echo "<tr>
            <td><a href=bossa_admin.php?action=show_batch&batch_id=$batch->id>$batch->id</a></td>
            <td>$batch->name</td>
            <td>$cal</td>
            <td>$t</td>
            <td>$n</td>
            <td>$c</td>
            </tr>
        ";
    }
    end_table();
    page_tail();
}

function show_insts($job_id) {
    $job = BossaJob::lookup_id($job_id);
    include_app_file($job->app_id);
    page_head("Instances of job $job_id");
    show_instances($job_id);
    page_tail();
}


$user = get_logged_in_user();

$db = BossaDb::get();
if (!$db) error_page("Can't connect to database server");

if (0) {
if (!$db->table_exists('bossa_app')) {
    page_head("Create Bossa database");
    $db_name = $db->db_name;
    echo "
        The database tables for Bossa don't seem to exist.
        To create them, go to ~/boinc/db and type
        <pre>
mysql $db_name < bossa_schema.sql
</pre>
    Then <a href=bossa_admin.php>reload this page</a>.
    ";
    page_tail();
    exit();
}
}

BossaUser::lookup($user);

$action = get_str('action', true);
switch ($action) {
case 'add_app':
    $name = BossaDb::escape_string(get_str('app_name'));
    $short_name = get_str('short_name');
    $description = BossaDb::escape_string(get_str('description'));
    $training_course = get_str('training_course' ,true);
    if (strlen($training_course)) {
        $course = BoltCourse::lookup_name($training_course);
        if (!$course) {
            error_page("No course named $training_course");
        }
        $courseid = $course->id;
    } else {
        $courseid = 0;
    }
    $calibration_frac = get_str('calibration_frac' ,true);
    $now = time();
    $app_id = BossaApp::insert("(create_time, name, short_name, description, bolt_course_id) values ($now, '$name', '$short_name', '$description', $courseid)");
    if ($courseid) {
        $course->update("bossa_app_id=$app_id");
    }
    Header('Location: bossa_admin.php');
    exit();
case 'update_user':
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bossa->update("flags=$flags");
    $user->bossa->flags = $flags;
    Header('Location: bossa_admin.php');
    exit();
case 'show_batches':
    $app_id = $_GET['app_id'];
    show_batches($app_id);
    exit();
case 'show_batch':
    $batch_id = $_GET['batch_id'];
    show_batch($batch_id);
    exit();
case 'show_insts':
    $job_id = $_GET['job_id'];
    show_insts($job_id);
    exit();
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
case '':
    show_all();
    exit();
default:
    error_page("unknown action $action");
}
Header('Location: bossa_admin.php');


?>
