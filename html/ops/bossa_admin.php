<?php

require_once("../inc/bossa_db.inc");
require_once("../inc/util_ops.inc");

function show_bapp($app) {
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
        echo "<br>";
        show_button($app->short_name."_workgen.php?njobs=10", "Create jobs", "Create 10 new jobs");
    }
}

function show_apps() {
    $apps = BossaApp::enum();
    start_table();
    row1("Existing apps", 2);
    table_header("Name/description", "");
    foreach ($apps as $app) {
        show_bapp($app);
    }
    end_table();
}

function add_app_form() {
    echo "
        <form action=bossa_admin.php method=get>
    ";
    start_table();
    row1("Add app");
    row2("Name<span class=note><br>Visible to users</span>", "<input name=app_name>");
    row2("Short name<span class=note><br>Used in file and function names - no spaces or special characters</span>", "<input name=short_name>");
    row2("Description<span class=note><br>Visible to users</span>", "<textarea name=description cols=60></textarea>");
    row2("Min confidence sum for consensus", "<input name=min_conf_sum value=2>");
    row2("Min confidence fraction for consensus", "<input name=min_conf_frac value=\"0.5\">");
    row2("Max job instances", "<input name=max_instances value=5>");
    row2("", "<input type=submit name=submit value=\"Create app\">");
    end_table();
    echo "</form>";
}

function user_settings() {
    global $user;
    $flags = $user->bossa->flags;
    echo "<form action=bossa_admin.php method=get>";
    start_table();
    row1("User settings");
    $x = ($flags&BOLT_FLAGS_SHOW_ALL)?"checked":"";
    row2("Show hidden apps?", "<input type=checkbox name=show_all $x>");
    $x = ($flags&BOLT_FLAGS_DEBUG)?"checked":"";
    row2("Show debugging output?", "<input type=checkbox name=debug $x>");
    row2("", "<input type=submit name=submit value=\"Update user\">");
    end_table();
    echo "</form>";
}

function show_all() {
    admin_page_head("Bossa app administration");
    show_apps();
    echo "<p>";
    add_app_form();
    echo "<p>";
    user_settings();
    admin_page_tail();
}

function show_jobs($app_id) {
    $app = BossaApp::lookup_id($app_id);
    echo "<h2>Jobs for $app->user_friendly_name</h2>";
    $jobs = BossaJob::enum("app_id=$app_id");
    foreach ($jobs as $job) {
        echo "<pre>\n";
        print_r($job);
        echo "</pre>\n";
        echo "
            <a href=bossa_admin.php?cmd=show_insts&job_id=$job->id>Show instances</a>
            <hr>
        ";
    }
}

function show_insts($job_id) {
    echo "<h2>Job instances</h2>";
    $jis = BossaJobInst::enum("job_id=$job_id");
    foreach ($jis as $ji) {
        echo "<pre>\n";
        print_r($ji);
        echo "</pre><hr>\n";
    }
}


$user = get_logged_in_user();

$submit = get_str('submit', true);
if ($submit == 'Create app') {
    $name = BossaDb::escape_string(get_str('app_name'));
    $short_name = get_str('short_name');
    $description = BossaDb::escape_string(get_str('description'));
    $min_conf_sum = get_str('min_conf_sum');
    $min_conf_frac = get_str('min_conf_frac');
    $max_instances = get_str('max_instances');
    $now = time();
    BossaApp::insert("(create_time, name, short_name, description, min_conf_sum, min_conf_frac, max_instances) values ($now, '$name', '$short_name', '$description', $min_conf_sum, $min_conf_frac, $max_instances)");
    Header('Location: bossa_admin.php');
    exit();
} else if ($submit == 'Update user') {
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bossa->update("flags=$flags");
    $user->bossa->flags = $flags;
    Header('Location: bossa_admin.php');
    exit();
} else if ($cmd == 'show_jobs') {
    $app_id = $_GET['app_id'];
    show_jobs($app_id);
} else if ($cmd == 'show_insts') {
    $job_id = $_GET['job_id'];
    show_insts($job_id);
} else {
    $action = get_str('action', true);
    if ($action) {
        $app_id = get_int('app_id');
        $app = BoltApp::lookup_id($app_id);
        if (!$app) error_page("no such app");
        switch ($action) {
        case 'hide':
            $app->update("hidden=1");
            break;
        case 'unhide':
            $app->update("hidden=0");
            break;
        default:
            error_page("unknown action $action");
        }
        Header('Location: bossa_admin.php');
        exit();
    }
}

show_all();

?>
