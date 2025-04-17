<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// web interface for managing BUDA science apps
//
// in the following, 'app' means BUDA science app
// and 'variant' means a variant of one of these (e.g. CPU, GPU)

require_once('../inc/util.inc');
require_once('../inc/sandbox.inc');
require_once('../inc/submit_util.inc');

display_errors();

$buda_root = "../../buda_apps";

// scan BUDA apps and variants, and write a file 'buda_plan_classes'
// in the project dir with list of plan classes
//
function write_plan_class_file() {
    $pcs = [];
    global $buda_root;
    if (is_dir($buda_root)) {
        $apps = scandir($buda_root);
        foreach ($apps as $app) {
            if ($app[0] == '.') continue;
            if (!is_dir("$buda_root/$app")) continue;
            $vars = scandir("$buda_root/$app");
            foreach ($vars as $var) {
                if ($var[0] == '.') continue;
                if (!is_dir("$buda_root/$app/$var")) continue;
                $pcs[] = $var;
            }
        }
    }
    $pcs = array_unique($pcs);
    file_put_contents(
        "../../buda_plan_classes",
        implode("\n", $pcs)."\n"
    );
}

// show list of BUDA apps and variants,
// w/ buttons for adding and deleting
//
function app_list($notice=null) {
    global $buda_root;
    if (!is_dir($buda_root)) {
        mkdir($buda_root);
    }
    page_head('BOINC Universal Docker App (BUDA)');
    if ($notice) {
        echo "$notice <p>\n";
    }
    text_start();
    echo "
        <p>BUDA lets you submit Docker jobs using a web interface.
        <a href=https://github.com/BOINC/boinc/wiki/BUDA-overview>Learn more</a>.
        <p>
        BUDA science apps:
    ";

    $dirs = scandir($buda_root);
    foreach ($dirs as $dir) {
        if ($dir[0] == '.') continue;
        panel("$dir",
            function() use ($dir) {
                show_app($dir);
            }
        );
    }
    show_button_small('buda.php?action=app_form', 'Add science app');
    text_end();
    page_tail();
}

function show_app($dir) {
    global $buda_root;
    start_table('table-striped');
    table_header('Variant<br><small>click for details</small>', 'Submit jobs');
    $pcs = scandir("$buda_root/$dir");
    $have_var = false;
    foreach ($pcs as $pc) {
        if ($pc[0] == '.') continue;
        $have_var = true;
        table_row(
            "<a href=buda.php?action=variant_view&app=$dir&variant=$pc>$pc</href>",
            button_text(
                "buda_submit.php?app=$dir&variant=$pc", "Submission form"
            )
        );
    }
    end_table();
    echo "<p>";
    show_button_small("buda.php?action=variant_form&app=$dir", 'Add variant');
    if (!$have_var) {
        echo "<p>";
        show_button(
            "buda.php?action=app_delete&app=$dir",
            "Delete app",
            null,
            'btn btn-xs btn-warning'
        );
    }
}

function file_row($app, $variant, $dir, $f) {
    [$md5, $size] = parse_info_file("$dir/.md5/$f");
    table_row(
        "<a href=buda.php?action=view_file&app=$app&variant=$variant&fname=$f>$f</a>",
        $size,
        $md5
    );
}

function variant_view() {
    global $buda_root;
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');
    page_head("BUDA: variant '$variant' of science app '$app'");
    $dir = "$buda_root/$app/$variant";
    $desc = json_decode(file_get_contents("$dir/variant.json"));
    start_table();
    table_header('Dockerfile', 'size', 'md5');
    file_row($app, $variant, $dir, $desc->dockerfile);
    table_header('App files', '', '');
    foreach ($desc->app_files as $f) {
        file_row($app, $variant, $dir, $f);
    }
    table_header('Auto-generated files', '', '');
    file_row($app, $variant, $dir, 'template_in');
    file_row($app, $variant, $dir, 'template_out');
    file_row($app, $variant, $dir, 'variant.json');
    end_table();

    start_table();
    row2(
        'Input filenames:',
        implode(',', $desc->input_file_names)
    );
    row2(
        'Output filenames:',
        implode(',', $desc->output_file_names)
    );
    if (!empty($desc->max_total)) {
        row2('Max total instances per job:', $desc->max_total);
    } else {
        row2('Max total instances per job:', '1');
    }
    if (!empty($desc->min_nsuccess)) {
        row2('Target successful instances per job:', $desc->min_nsuccess);
    } else {
        row2('Target successful instances per job:', '1');
    }
    end_table();

    echo '<p>';
    show_button(
        "buda.php?action=variant_delete&app=$app&variant=$variant",
        'Delete variant',
        null,
        'btn btn-xs btn-warning'
    );
    page_tail();
}

// form for creating app variant.
// Currently doesn't support indirect files.
// doing this would require checkboxes for indirect
//
// Could have other stuff like
//      - min_quorum, max_total_results
//      - rsc_disk_bound, rsc_memory_bound
// or does this belong in job submission?
//
function variant_form($user) {
    $sbitems = sandbox_select_items($user);
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');

    page_head_select2("Create a variant of Docker app $app");
    echo "
        Details are <a href=https://github.com/BOINC/boinc/wiki/BUDA-job-submission#adding-a-variant>here</a>.
    ";
    $sb = '<br><small>From your <a href=sandbox.php>file sandbox</a></small>';
    $pc = '<br><small>Specify
    <a href=https://github.com/BOINC/boinc/wiki/AppPlan>GPU and other requirements</a>';
    form_start('buda.php');
    form_input_hidden('app', $app);
    form_input_hidden('action', 'variant_action');
    form_input_text("Plan class$pc", 'variant');
    form_select("Dockerfile$sb", 'dockerfile', $sbitems);
    form_select2_multi("Application files$sb", 'app_files', $sbitems, null);
    form_input_text(
        'Input file names<br><small>Space-separated</small>',
        'input_file_names'
    );
    form_input_text(
        'Output file names<br><small>Space-separated</small>',
        'output_file_names'
    );
    form_input_text(
        'Run at most this many total instances of each job',
        'max_total',
        '1'
    );
    form_input_text(
        'Get this many successful instances of each job
            <br><small>(subject to the above limit)</small>
        ',
        'min_nsuccess',
        '1'
    );
    form_submit('OK');
    form_end();
    page_tail();
}

function buda_file_phys_name($app, $variant, $md5) {
    return sprintf('buda_%s_%s_%s', $app, $variant, $md5);
}

// copy file from sandbox to variant dir, and stage to download hier
//
function copy_and_stage_file($user, $fname, $dir, $app, $variant) {
    copy_sandbox_file($user, $fname, $dir);
    [$md5, $size] = parse_info_file("$dir/.md5/$fname");
    $phys_name = buda_file_phys_name($app, $variant, $md5);
    stage_file_aux("$dir/$fname", $md5, $size, $phys_name);
    return $phys_name;
}

// create templates and put them in variant dir
//
function create_templates($variant, $variant_desc, $dir) {
    // input template
    //
    $x = "<input_template>\n";
    $ninfiles = 1 + count($variant_desc->input_file_names) + count($variant_desc->app_files);
    for ($i=0; $i<$ninfiles; $i++) {
        $x .= "   <file_info>\n      <sticky/>\n      <no_delete/>\n      <executable/>\n   </file_info>\n";
    }
    $x .= "   <workunit>\n";
    $x .= file_ref_in($variant_desc->dockerfile);
    foreach ($variant_desc->app_files as $fname) {
        $x .= file_ref_in($fname);
    }
    foreach ($variant_desc->input_file_names as $fname) {
        $x .= file_ref_in($fname);
    }
    if ($variant == 'cpu') {
        $x .= "      <plan_class></plan_class>\n";
    } else {
        $x .= "      <plan_class>$variant</plan_class>\n";
    }

    // replication params
    //
    $x .= sprintf("      <target_nresults>%d</target_nresults>\n",
        $variant_desc->min_nsuccess
    );
    $x .= sprintf("      <min_quorum>%d</min_quorum>\n",
        $variant_desc->min_nsuccess
    );
    $x .= sprintf("      <max_total_results>%d</max_total_results>\n",
        $variant_desc->max_total
    );

    $x .= "   </workunit>\n<input_template>\n";
    file_put_contents("$dir/template_in", $x);

    // output template
    //
    $x = "<output_template>\n";
    $i = 0;
    foreach ($variant_desc->output_file_names as $fname) {
        $x .= file_info_out($i++);
    }
    $x .= "   <result>\n";
    $i = 0;
    foreach ($variant_desc->output_file_names as $fname) {
        $x .= file_ref_out($i++, $fname);
    }
    $x .= "   </result>\n</output_template>\n";
    file_put_contents("$dir/template_out", $x);
}

// create variant
//
function variant_action($user) {
    global $buda_root;
    $variant = get_str('variant');
    if (!$variant) $variant = 'cpu';
    if (!is_valid_filename($variant)) {
        error_page(filename_rules());
    }
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $dockerfile = get_str('dockerfile');
    if (!is_valid_filename($dockerfile)) {
        error_page("Invalid dockerfile name: ".filename_rules());
    }
    $app_files = get_array('app_files');
    foreach ($app_files as $fname) {
        if (!is_valid_filename($fname)) {
            error_page("Invalid app file name: ".filename_rules());
        }
    }
    $min_nsuccess = get_int('min_nsuccess');
    if ($min_nsuccess <= 0) {
        error_page('Must specify a positive number of successful instances.');
    }
    $max_total = get_int('max_total');
    if ($max_total <= 0) {
        error_page('Must specify a positive max number of instances.');
    }
    if ($min_success > $max_total) {
        error_page('Target # of successful instances must be <= max total');
    }
    $input_file_names = get_str('input_file_names', true);
    $output_file_names = explode(' ', get_str('output_file_names'));
    if ($input_file_names) {
        $input_file_names = explode(' ', $input_file_names);
        foreach ($input_file_names as $fname) {
            if (!is_valid_filename($fname)) {
                error_page("Invalid input file name: ".filename_rules());
            }
        }
    } else {
        $input_file_names = [];
    }
    foreach ($output_file_names as $fname) {
        if (!is_valid_filename($fname)) {
            error_page("Invalid output file name: ".filename_rules());
        }
    }

    if (file_exists("$buda_root/$app/$variant")) {
        error_page("Variant '$variant' already exists.");
    }
    $dir = "$buda_root/$app/$variant";
    mkdir($dir);
    mkdir("$dir/.md5");

    // collect variant params into a struct
    //
    $desc = new StdClass;
    $desc->dockerfile = $dockerfile;
    $desc->app_files = $app_files;
    $desc->input_file_names = $input_file_names;
    $desc->output_file_names = $output_file_names;
    $desc->min_nsuccess = $min_nsuccess;
    $desc->max_total = $max_total;

    // copy files from sandbox to variant dir
    //
    $pname = copy_and_stage_file($user, $dockerfile, $dir, $app, $variant);
    $desc->dockerfile_phys = $pname;
    $desc->app_files_phys = [];
    foreach ($app_files as $fname) {
        $pname = copy_and_stage_file($user, $fname, $dir, $app, $variant);
        $desc->app_files_phys[] = $pname;
    }

    // write variant params to a JSON file
    //
    file_put_contents(
        "$dir/variant.json",
        json_encode($desc, JSON_PRETTY_PRINT)
    );

    create_templates($variant, $desc, $dir);

    // Note: we don't currently allow indirect file access.
    // If we did, we'd need to create job.toml to mount project dir

    app_list("Variant $variant added for app $app.");
}

function variant_delete() {
    global $buda_root;
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');
    $confirmed = get_str('confirmed', true);
    if ($confirmed) {
        $dir = "$buda_root/$app/$variant";
        if (!file_exists($dir)) error_page('no such variant');
        // delete staged files
        //
        foreach (scandir("$dir/.md5") as $fname) {
            if ($fname[0] == '.') continue;
            [$md5, $size] = parse_info_file("$dir/.md5/$fname");
            $phys_name = buda_file_phys_name($app, $variant, $md5);
            $phys_path = download_hier_path($phys_name);
            unlink($phys_path);
            unlink("$phys_path.md5");
        }
        system("rm -r $buda_root/$app/$variant", $ret);
        if ($ret) {
            error_page("delete failed");
        }
        $notice = "Variant $variant of app $app removed.";
        app_list($notice);
    } else {
        page_head("Confirm");
        echo "<p>Are you sure you want to delete variant $variant of BUDA app $app?  <p>";
        show_button(
            "buda.php?action=variant_delete&app=$app&variant=$variant&confirmed=yes",
            "Yes"
        );
        page_tail();
    }
}

function app_delete() {
    global $buda_root;
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $confirmed = get_str('confirmed', true);
    if ($confirmed) {
        $dir = "$buda_root/$app";
        if (!file_exists($dir)) error_page('no such app');
        foreach (scandir($dir) as $fname) {
            if ($fname[0] == '.') continue;
            error_page("You must delete all variants first.");
        }
        system("rmdir $buda_root/$app", $ret);
        if ($ret) {
            error_page('delete failed');
        }
        $notice = "App $app removed.";
        app_list($notice);
    } else {
        page_head('Confirm');
        echo "<p>Are you sure you want to delete BUDA science app $app?  <p>";
        show_button(
            "buda.php?action=app_delete&app=$app&confirmed=yes",
            "Yes"
        );
        page_tail();
    }
}

function app_form() {
    page_head('Create Docker app');
    form_start('buda.php');
    form_input_hidden('action', 'app_action');
    form_input_text('Name', 'name');
    form_submit('OK');
    form_end();
    page_tail();
}

function app_action() {
    global $buda_root;
    $name = get_str('name');
    if (!is_valid_filename($name)) {
        error_page(filename_rules());
    }
    $dir = "$buda_root/$name";
    if (file_exists($dir)) {
        error_page("App $name already exists.");
    }
    mkdir($dir);
    header("Location: buda.php");
}

function view_file() {
    global $buda_root;
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');
    $fname = get_str('fname');
    if (!is_valid_filename($fname)) die('bad arg');
    echo "<pre>\n";
    $x = file_get_contents("$buda_root/$app/$variant/$fname");
    echo htmlspecialchars($x);
    echo "</pre>\n";
}

// check access.
// Anyone with submit access to BUDA can add/delete apps and variants.
// Might want to refine this at some point

$user = get_logged_in_user();
$buda_app = BoincApp::lookup("name='buda'");
if (!$buda_app) error_page('no buda app');
if (!has_submit_access($user, $buda_app->id)) {
    error_page('no access');
}

$action = get_str('action', true);
switch ($action) {
case 'app_form':
    app_form(); break;
case 'app_action':
    app_action(); break;
case 'app_delete':
    app_delete(); break;
case 'variant_view':
    variant_view($user); break;
case 'variant_form':
    variant_form($user); break;
case 'variant_action':
    variant_action($user);
    write_plan_class_file();
    break;
case 'variant_delete':
    variant_delete();
    write_plan_class_file();
    break;
case 'view_file':
    view_file(); break;
case null:
    app_list(); break;
default:
    error_page("unknown action");
}

?>
