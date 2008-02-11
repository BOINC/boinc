<?php

require_once("../inc/bossa_db.inc");
require_once("../inc/util_ops.inc");

function show_bapp($app) {
    echo "<tr>
        <td>Name: $app->name<br>Description: $app->description<br>Created: ".date_str($app->create_time)."</td>
        <td>$app->display_script</td>
        <td>$app->backend_script</td>
        <td>
    ";
    if ($app->hidden) {
        show_button("bossa_ops.php?action=unhide&app_id=$app->id", "Unhide", "Unhide this app");
    } else {
        show_button("bossa_ops.php?action=hide&app_id=$app->id", "Hide", "Hide this app");
    }
}

function show_apps() {
    $apps = BossaApp::enum();
    start_table();
    row1("Existing apps", 4);
    table_header("Name/description", "Display script", "Backend script", "");
    foreach ($apps as $app) {
        show_bapp($app);
    }
    end_table();
}

function add_app_form() {
    echo "
        <form action=bossa_ops.php method=get>
    ";
    start_table();
    row1("Add app");
    row2("Name<span class=note><br>Visible to users</span>", "<input name=app_name>");
    row2("Description<span class=note><br>Visible to users</span>", "<textarea name=description cols=60></textarea>");
    row2("Display script filename", "<input name=display_script>");
    row2("Backend script filename", "<input name=backend_script>");
    row2("Min confidence sum for consensus", "<input name=min_conf_sum>");
    row2("Min confidence fraction for consensus", "<input name=min_conf_frac>");
    row2("Max instances", "<input name=max_instances>");
    row2("", "<input type=submit name=submit value=\"Create app\">");
    end_table();
    echo "</form>";
}

function user_settings() {
    global $user;
    $flags = $user->bossa->flags;
    echo "<form action=bossa_ops.php method=get>";
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

$user = get_logged_in_user();

$submit = get_str('submit', true);
if ($submit == 'Create app') {
    $name = BossaDb::escape_string(get_str('app_name'));
    $description = BossaDb::escape_string(get_str('description'));
    $display_script = get_str('display_script');
    $backend_script = get_str('backend_script');
    $min_conf_sum = get_str('min_conf_sum');
    $min_conf_frac = get_str('min_conf_frac');
    $max_instances = get_str('max_instances');
    $now = time();
    BossaApp::insert("(create_time, name, description, display_script, backend_script, min_conf_sum, min_conf_frac, max_instances) values ($now, '$name', '$description', '$display_script', '$backend_script', $min_conf_sum, $min_conf_frac, $max_instances)");
    Header('Location: bossa_ops.php');
    exit();
} else if ($submit == 'Update user') {
    $flags = 0;
    if (get_str('show_all', true)) $flags |= BOLT_FLAGS_SHOW_ALL;
    if (get_str('debug', true)) $flags |= BOLT_FLAGS_DEBUG;
    $user->bossa->update("flags=$flags");
    $user->bossa->flags = $flags;
    Header('Location: bossa_ops.php');
    exit();
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
        Header('Location: bossa_ops.php');
        exit();
    }
}

show_all();

?>
