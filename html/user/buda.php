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

// interface for:
//      - viewing details of BUDA science apps and variants
//      - managing these if user has permission
//
// in the following, 'app' means BUDA science app
// and 'variant' means a variant of one of these (e.g. CPU, GPU)

require_once('../inc/util.inc');
require_once('../inc/sandbox.inc');
require_once('../inc/keywords.inc');
require_once('../inc/submit_util.inc');
require_once('../inc/buda.inc');

display_errors();

$buda_root = "../../buda_apps";

// show list of BUDA apps and variants,
// w/ buttons for adding and deleting
//
function app_list($notice=null) {
    global $buda_root;
    if (!is_dir($buda_root)) {
        mkdir($buda_root);
    }
    page_head('Manage BUDA apps');
    if ($notice) {
        echo "$notice <p>\n";
    }
    text_start();
    echo "
        <p>BUDA lets you submit Docker jobs using a web interface.
        <a href=https://github.com/BOINC/boinc/wiki/BUDA-overview>Learn more</a>.
        <p>
        <h3>BUDA science apps</h3>
    ";

    $apps = get_buda_apps();
    foreach ($apps as $app) {
        show_app($app);
    }
    echo '<hr>';
    show_button_small('buda.php?action=app_form', 'Add science app');
    text_end();
    page_tail();
}

function show_app($app_dir) {
    global $buda_root;
    $desc = null;
    $desc_path = "$buda_root/$app_dir/desc.json";
    $desc = json_decode(file_get_contents($desc_path));
    echo '<hr>';
    echo sprintf('<h3>%s</h3><p>', $desc->long_name);
    show_button_small(
        sprintf('buda.php?action=app_details&name=%s', $desc->name),
        'App details'
    );
    $var_dirs = get_buda_variants($app_dir);
    if ($var_dirs) {
        echo "<p>Variants:<ul>";
        foreach ($var_dirs as $var_dir) {
            $var_desc = get_buda_var_desc($app_dir, $var_dir);
            echo sprintf(
                '<li><a href=buda.php?action=variant_view&app=%s&variant=%s>%s</a>',
                $app_dir, $var_dir, variant_name($var_desc)
            );
        }
        echo '</ul>';
    } else {
        echo '<p>No variants';
    }
    echo "<p>";
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
    global $buda_root, $manage_access;
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');
    $variant = get_str('variant');
    if (!is_valid_filename($variant)) die('bad arg');
    page_head("BUDA variant");
    $dir = "$buda_root/$app/$variant";
    $desc = json_decode(file_get_contents("$dir/variant.json"));
    start_table('table-striped');
    row2("BUDA App", $app);
    row2("Variant name", $variant);
    row2("CPU type", $desc->cpu_type);
    row2("Plan class", $desc->plan_class);
    end_table();
    echo "<h3>App files</h3>";
    start_table();
    table_header('Dockerfile', 'size', 'md5');
    file_row($app, $variant, $dir, $desc->dockerfile);
    table_header('App files', '', '');
    foreach ($desc->app_files as $f) {
        file_row($app, $variant, $dir, $f);
    }
    table_header('Auto-generated files', '', '');
    file_row($app, $variant, $dir, 'variant.json');
    end_table();
    echo '<hr>';

    if ($manage_access) {
        echo '<p>';
        show_button_small(
            "buda.php?action=variant_form&app=$app&variant=$variant",
            'Edit variant'
        );
        echo '<p>';
        show_button(
            "buda.php?action=variant_delete&app=$app&variant=$variant",
            'Delete variant',
            null,
            'btn btn-xs btn-warning'
        );
    }
    page_tail();
}

// form for creating an app variant or editing an existing one
//
function variant_form($user) {
    $sbitems = sandbox_select_items($user);
    $app = get_str('app');
    if (!is_valid_filename($app)) die('bad arg');

    // name of variant directory, if we're editing
    $variant = get_str('variant', true);

    if ($variant) {
        global $buda_root;
        $variant_dir = "$buda_root/$app/$variant";
        $variant_desc = json_decode(
            file_get_contents("$variant_dir/variant.json")
        );
        if (!$variant_desc) error_page('no such variant');
        page_head_select2("Edit variant $variant of BUDA app $app");
    } else {
        $variant_desc = new StdClass;
        $variant_desc->cpu_type = 'intel';
        $variant_desc->plan_class = '';
        $variant_desc->dockerfile = '';
        $variant_desc->app_files = [];
        page_head_select2("Create a variant of BUDA app $app");
    }
    echo "
        Details are <a href=https://github.com/BOINC/boinc/wiki/BUDA-job-submission#adding-a-variant>here</a>.
    ";
    $sb = '<br><small>From your <a href=sandbox.php>file sandbox</a></small>';
    $pc = '<br><small>Specify
    <a href=https://github.com/BOINC/boinc/wiki/AppPlan>GPU and other host requirements</a>.<br>Leave blank if none.</small>';
    form_start('buda.php');
    form_input_hidden('app', $app);
    form_input_hidden('action', 'variant_action');
    if ($variant) {
        // can't change CPU type of existing variant
        form_input_hidden('variant', $variant);
        form_input_hidden('edit', 'true');
        $x = explode('_', $variant);
        $cpu_type = $x[0];
        form_input_hidden('cpu_type', $cpu_type);
    } else {
        form_radio_buttons(
            'CPU type', 'cpu_type',
            [
                ['intel', 'Intel'],
                ['arm', 'ARM']
            ],
            $variant_desc->cpu_type
        );
    }
    form_input_text("Plan class$pc", 'plan_class', $variant_desc->plan_class);
    form_select("Dockerfile$sb", 'dockerfile', $sbitems, $variant_desc->dockerfile);
    form_select2_multi("Application files$sb", 'app_files', $sbitems, $variant_desc->app_files);
    form_submit('OK');
    form_end();
    page_tail();
}

function buda_file_phys_name($app, $variant, $md5) {
    return sprintf('buda_%s_%s_%s', $app, $variant, $md5);
}

// copy file from sandbox to variant dir, and stage to download hier
// return physical name, md5, and size
//
function copy_and_stage_file($user, $fname, $variant_dir, $app, $variant) {
    copy_sandbox_file($user, $fname, $variant_dir);
    [$md5, $size] = parse_info_file("$variant_dir/.md5/$fname");
    $phys_name = buda_file_phys_name($app, $variant, $md5);
    stage_file_aux("$variant_dir/$fname", $md5, $size, $phys_name);
    return [$phys_name, $md5, $size];
}

// create templates and put them in app dir
//
function create_templates($app, $desc, $dir) {
    // input template
    //
    $x = "<input_template>\n";
    $ninfiles = count($desc->input_file_names);
    for ($i=0; $i<$ninfiles; $i++) {
        $x .= "   <file_info>\n      <no_delete/>\n   </file_info>\n";
    }
    $x .= "   <workunit>\n";
    foreach ($desc->input_file_names as $fname) {
        $x .= file_ref_in($fname);
    }

    // replication params
    //
    $x .= sprintf("      <target_nresults>%d</target_nresults>\n",
        $desc->min_nsuccess
    );
    $x .= sprintf("      <min_quorum>%d</min_quorum>\n",
        $desc->min_nsuccess
    );
    $x .= sprintf("      <max_total_results>%d</max_total_results>\n",
        $desc->max_total
    );

    $x .= sprintf("      <max_delay>%f</max_delay>\n",
        $desc->max_delay_days * 86400.
    );

    $x .= "      <buda_app_name>$app</buda_app_name>\n";
    $x .= "   </workunit>\n</input_template>\n";
    file_put_contents("$dir/template_in", $x);

    // output template
    //
    $x = "<output_template>\n";
    $i = 0;
    foreach ($desc->output_file_names as $fname) {
        $x .= file_info_out($i++);
    }
    $x .= "   <result>\n";
    $i = 0;
    foreach ($desc->output_file_names as $fname) {
        $x .= file_ref_out($i++, $fname);
    }
    $x .= "   </result>\n</output_template>\n";
    file_put_contents("$dir/template_out", $x);
}

// return <file_info> and <file_ref> elements for given file
//
function file_xml_elements($log_name, $phys_name, $md5, $nbytes) {
    static $download_dir, $download_url, $fanout;
    if ($download_dir == null) {
        $download_dir = parse_element(get_config(), "<download_dir>");
        $download_url = parse_element(get_config(), "<download_url>");
        $fanout = (int)(parse_element(get_config(), "<uldl_dir_fanout>"));
    }
    $file_info = sprintf(
"<file_info>
    <sticky/>
    <no_delete/>
    <executable/>
    <name>%s</name>
    <url>%s/%s/%s</url>
    <md5_cksum>%s</md5_cksum>
    <nbytes>%d</nbytes>
</file_info>
",
        $phys_name,
        $download_url, filename_hash($phys_name, $fanout), $phys_name,
        $md5,
        $nbytes
    );
    $file_ref = sprintf(
"<file_ref>
    <file_name>%s</file_name>
    <open_name>%s</open_name>
    <copy_file/>
</file_ref>
",
        $phys_name,
        $log_name
    );
    return [$file_info, $file_ref];
}

// create variant
//
function variant_action($user) {
    global $buda_root;
    $cpu_type = get_str('cpu_type');
    $plan_class = get_str('plan_class');
    $variant = get_str('variant', true);
    if ($variant) {
        $creating = false;
    } else {
        $creating = true;
    }
    $variant = sprintf('%s_%s', $cpu_type, $plan_class?$plan_class:'cpu');
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

    $dir = "$buda_root/$app/$variant";
    if ($creating) {
        if (file_exists($dir)) {
            error_page("Variant '$variant' already exists.");
        }
    } else {
        system("rm -r $dir");
    }
    mkdir($dir);
    mkdir("$dir/.md5");

    // collect variant params into a struct
    //
    $desc = new StdClass;
    $desc->dockerfile = $dockerfile;
    $desc->app_files = $app_files;
    $desc->cpu_type = $cpu_type;
    $desc->plan_class = $plan_class;
    $desc->file_infos = '';
    $desc->file_refs = '';

    // copy dockerfile and app files from sandbox to variant dir,
    // and stage them to download dir
    //
    [$pname, $md5, $nbytes] = copy_and_stage_file(
        $user, $dockerfile, $dir, $app, $variant
    );
    $desc->dockerfile_phys = $pname;
    $desc->app_files_phys = [];
    [$file_info, $file_ref] = file_xml_elements(
        'Dockerfile', $pname, $md5, $nbytes
    );
    $desc->file_infos .= $file_info;
    $desc->file_refs .= $file_ref;

    foreach ($app_files as $fname) {
        [$pname, $md5, $nbytes] = copy_and_stage_file(
            $user, $fname, $dir, $app, $variant
        );
        $desc->app_files_phys[] = $pname;
        [$file_info, $file_ref] = file_xml_elements(
            $fname, $pname, $md5, $nbytes
        );
        $desc->file_infos .= $file_info;
        $desc->file_refs .= $file_ref;
    }

    // write variant params to a JSON file
    //
    file_put_contents(
        "$dir/variant.json",
        json_encode($desc, JSON_PRETTY_PRINT)
    );

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
        $vars = get_buda_variants($app);
        if ($vars) {
            error_page("You must delete all variants first.");
        }
        system("rm $buda_root/$app/desc.json", $ret);
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

function app_form($desc=null) {
    page_head_select2($desc?"Edit BUDA app $desc->name":'Create BUDA app');
    form_start('buda.php');
    form_input_hidden('action', 'app_action');
    if ($desc) {
        form_input_hidden('edit_name', $desc->name);
        form_input_hidden('user_id', $desc->user_id);
        form_input_hidden('create_time', $desc->create_time);
    } else {
        $desc = new StdClass;
        $desc->long_name = null;
        $desc->input_file_names = [];
        $desc->output_file_names = [];
        $desc->min_nsuccess = 1;
        $desc->max_total = 2;
        $desc->max_delay_days = 7;
        $desc->description = null;
        $desc->sci_kw = null;
        $desc->url = null;
        form_input_text('Internal name<br><small>No spaces</small>', 'name');
    }
    form_input_text('User-visible name', 'long_name', $desc->long_name);
    form_input_text(
        'Input file names<br><small>Space-separated</small>',
        'input_file_names',
        implode(' ', $desc->input_file_names)
    );
    form_input_text(
        'Output file names<br><small>Space-separated</small>',
        'output_file_names',
        implode(' ', $desc->output_file_names)
    );
    form_input_text(
        'Run at most this many total instances of each job',
        'max_total',
        $desc->max_total
    );
    form_input_text(
        'Get this many successful instances of each job
            <br><small>(subject to the above limit)</small>
        ',
        'min_nsuccess',
        $desc->min_nsuccess
    );
    form_input_text(
        'Max job turnaround time, days',
        'max_delay_days',
        $desc->max_delay_days
    );
    form_input_textarea(
        'Description<br><small>... of what the app does and of the research goals</small>',
        'description',
        $desc->description
    );
    form_select2_multi('Science keywords',
        'sci_kw',
        keyword_select_options(KW_CATEGORY_SCIENCE),
        $desc->sci_kw
    );
    // don't include location keywords;
    // various people may submit jobs to this app
    form_submit('OK');
    form_end();
    page_tail();
}

function app_action($user) {
    global $buda_root;
    $edit_name = get_str('edit_name', true);
    $desc = new StdClass;
    if ($edit_name) {
        // editing existing app
        $dir = "$buda_root/$edit_name";
        $app_name = $edit_name;
        $desc->user_id = get_int('user_id');
        $desc->create_time = get_int('create_time');
    } else {
        // creating new app
        $app_name = get_str('name');
        if (!is_valid_filename($app_name)) {
            error_page(filename_rules());
        }
        $dir = "$buda_root/$app_name";
        if (file_exists($dir)) {
            error_page("App $app_name already exists.");
        }
        mkdir($dir);
        $desc->user_id = $user->id;
        $desc->create_time = time();
    }
    $desc->name = $app_name;
    $min_nsuccess = get_int('min_nsuccess');
    if ($min_nsuccess <= 0) {
        error_page('Must specify a positive number of successful instances.');
    }
    $max_total = get_int('max_total');
    if ($max_total <= 0) {
        error_page('Must specify a positive max number of instances.');
    }
    if ($min_nsuccess > $max_total) {
        error_page('Target # of successful instances must be <= max total');
    }
    $max_delay_days = get_str('max_delay_days');
    if (!is_numeric($max_delay_days)) {
        error_page('Must specify max delay');
    }
    $max_delay_days = floatval($max_delay_days);
    if ($max_delay_days <= 0) {
        error_page('Must specify positive max delay');
    }

    $input_file_names = get_str('input_file_names', true);
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
    $output_file_names = get_str('output_file_names', true);
    if ($output_file_names) {
        $output_file_names = explode(' ', $output_file_names);
        foreach ($output_file_names as $fname) {
            if (!is_valid_filename($fname)) {
                error_page("Invalid output file name: ".filename_rules());
            }
        }
    } else {
        $output_file_names = [];
    }
    $desc->long_name = get_str('long_name');
    $desc->input_file_names = $input_file_names;
    $desc->output_file_names = $output_file_names;
    $desc->min_nsuccess = $min_nsuccess;
    $desc->max_total = $max_total;
    $desc->max_delay_days = $max_delay_days;
    $desc->description = get_str('description');
    $desc->sci_kw = array_map('intval', get_array('sci_kw'));
    file_put_contents("$dir/desc.json", json_encode($desc, JSON_PRETTY_PRINT));

    create_templates($app_name, $desc, $dir);

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

function handle_app_edit() {
    global $buda_root;
    $name = get_str('name');
    app_form(get_buda_app_desc($name));
}

function app_details() {
    global $buda_root, $manage_access;
    $name = get_str('name');
    $desc = get_buda_app_desc($name);
    if (!$desc) error_page("no desc file $path");
    page_head("BUDA app: $desc->long_name");
    start_table('table-striped');
    row2('Internal name', $desc->name);
    $user = BoincUser::lookup_id($desc->user_id);
    row2('Creator',
        sprintf('<a href=show_user.php?userid=%d>%s</a>',
            $user->id,
            $user->name
        )
    );
    row2('Created', date_str($desc->create_time));
    row2('Description', $desc->description);
    row2('Science keywords', kw_array_to_str($desc->sci_kw));
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
    if (!empty($desc->max_delay_days)) {
        row2('Max job turnaround time, days:', $desc->max_delay_days);
    } else {
        row2('Max job turnaround time, days:', '7');
    }
    if ($manage_access) {
        row2('',
            button_text_small(
                sprintf('buda.php?action=%s&name=%s', 'app_edit', $desc->name),
                'Edit app info'
            )
        );
    }
    $vars = get_buda_variants($name);
    if ($vars) {
        $x = [];
        foreach ($vars as $var) {
            $x[] = sprintf('<a href=buda.php?action=variant_view&app=%s&variant=%s>%s</a>',
                $name, $var, $var
            );
        }
        row2('Variants', implode('<p>', $x));
        if ($manage_access) {
            row2('',
                button_text_small(
                    "buda.php?action=variant_form&app=$name",
                    'Add variant'
                )
            );
        }
    } else if ($manage_access) {
        row2('Variants',
            button_text_small(
                "buda.php?action=variant_form&app=$name",
                'Add variant'
            )
        );
        row2('',
            button_text(
                "buda.php?action=app_delete&app=$name",
                "Delete app",
                null,
                'btn btn-xs btn-warning'
            )
        );
    }
    end_table();
    page_tail();
}

// Users with manage access to BUDA can add/delete apps and variants.
// Others can just view.
// Might want to refine this at some point

$user = get_logged_in_user();
$buda_app = BoincApp::lookup("name='buda'");
if (!$buda_app) error_page('no buda app');
$manage_access = has_manage_access($user, $buda_app->id);

$action = get_str('action', true);
switch ($action) {
case 'app_edit':
    if (!$manage_access) error_page('no access');
    handle_app_edit(); break;
case 'app_form':
    if (!$manage_access) error_page('no access');
    app_form(); break;
case 'app_action':
    if (!$manage_access) error_page('no access');
    app_action($user); break;
case 'app_details':
    app_details(); break;
case 'app_delete':
    if (!$manage_access) error_page('no access');
    app_delete(); break;
case 'variant_view':
    variant_view($user); break;
case 'variant_form':
    if (!$manage_access) error_page('no access');
    variant_form($user); break;
case 'variant_action':
    if (!$manage_access) error_page('no access');
    variant_action($user);
    break;
case 'variant_delete':
    if (!$manage_access) error_page('no access');
    variant_delete();
    break;
case 'view_file':
    view_file(); break;
case null:
    app_list(); break;
default:
    error_page("unknown action");
}

?>
