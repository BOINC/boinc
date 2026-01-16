<?php

require_once('../inc/util.inc');
require_once('../inc/buda.inc');
require_once('../inc/keywords.inc');
require_once('../project/remote_apps.inc');
require_once('../project/submitters.inc');

function show_submitters() {
    global $submitters;
    if (!$submitters) return;
    echo '<h2>Researchers</h2>';
    foreach ($submitters as $s) {
        if (empty($s->picture)) {
            $s->picture = 'images/user.png';
        }
        echo sprintf(
            '<img height=100px src=%s>
            <p>Name: %s
            <br>Research interests: %s
            <br>Location: %s
            <p>
            ',
            $s->picture,
            $s->name,
            $s->interests,
            kw_array_to_str($s->loc_kw, '; ')
        );
    }
}

function show_buda_apps() {
    $apps = get_buda_apps();
    foreach ($apps as $app) {
        $desc = get_buda_app_desc($app);
        table_row(
            $desc->long_name,
            $desc->description,
            kw_array_to_str($desc->sci_kw),
            empty($desc->url)?'':"<a href=$desc->url>View</a>",
            'Yes'
        );
    }
}

function main() {
    global $remote_apps;
    page_head("Apps");
    echo sprintf('%s distributes jobs for the following applications:<p>', PROJECT);
    start_table('table-striped');
    table_header(
        'Name',
        'Description',
        'Science keywords',
        'Web page',
        'Docker required?<br><small><a href=https://github.com/BOINC/boinc/wiki/Installing-Docker>Details</a>'
    );
    foreach ($remote_apps as $category => $apps) {
        if (strstr($category, 'Docker')) {
            show_buda_apps();
        } else {
            foreach ($apps as $app) {
                table_row(
                    $app->long_name,
                    $app->description,
                    kw_array_to_str($app->sci_kw),
                    "<a href=$app->url>View</a>",
                    ''
                );
            }
            echo '</ul>';
        }
    }
    end_table();
    $user = get_logged_in_user(false);
    if ($user) {
        $us = BoincUserSubmit::lookup_userid($user->id);
        if (!$us) {
            show_button("apply.php", "Apply to submit jobs");
        }
    } else {
        show_button("signup.php", "Register to submit jobs");
    }
    show_submitters();
    page_tail();
}

main();

?>
