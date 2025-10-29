<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// Interface for project-wide functions:
//   - control user quotas and permissions to submit jobs
//   - create apps (not implemented yet)

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/keywords.inc");
require_once("../inc/kw_prefs.inc");

function user_row($u) {
    $user = BoincUser::lookup_id($u->user_id);
    $uas = BoincUserSubmitApp::enum("user_id=$u->user_id");

    if ($u->submit_all) {
        $sub = 'All applications';
    } else {
        $names = [];
        foreach ($uas as $ua) {
            $app = BoincApp::lookup_id($ua->app_id);
            $names[] = $app->name;
        }
        $sub = $names?implode(', ', $names):'---';
    }
    if ($u->manage_all) {
        $admin = 'All applications';
    } else {
        $names = [];
        foreach ($uas as $ua) {
            if (!$ua->manage) continue;
            $app = BoincApp::lookup_id($ua->app_id);
            $names[] = $app->name;
        }
        $admin = $names?implode(', ', $names):'---';
    }
    [$yes, $no] = read_kw_prefs($user);
    global $job_keywords;
    $kws = [];
    foreach ($yes as $id) {
        $kw = $job_keywords[$id];
        $kws[] = $kw->name;
    }

    table_row(
        sprintf(
            '<a href=manage_project.php?action=edit_form&user_id=%d>%s</a>',
            $u->user_id, $user->name
        ),
        $sub,
        $admin,
        implode('<br>', $kws),
        $u->quota,
        $u->max_jobs_in_progress,
        ($u->logical_start_time > time())?local_time_str($u->logical_start_time):'---'
    );
}

function handle_list() {
    page_head("Job submission access control");
    echo "The following users are allowed to submit jobs.
        <p>
    ";

    $us = BoincUserSubmit::enum("");
    start_table('table-striped');
    table_header(
        "User<br><small>Click to change permissions or quota</small>",
        "Can submit jobs to",
        "Can administer apps for",
        'Keywords',
        "Quota",
        "Max jobs in progress<br><small>0 means no limit</small>",
        "Current priority<br><small>Later time = lower priority</small>"
    );
    foreach ($us as $u) {
        user_row($u);
    }
    end_table();
    show_button("manage_project.php?action=add_form",
        "Add user", "Allow a new user to submit jobs"
    );
    page_tail();
}

// get multi-select lists for apps
//
function get_app_lists($user_id) {
    $items = [];
    $submit = [];
    $manage = [];
    $apps = BoincApp::enum("deprecated=0");
    foreach ($apps as $app) {
        $items[] = [$app->id, $app->name];
        $us = BoincUserSubmitApp::lookup("user_id=$user_id and app_id=$app->id");
        if ($us) {
            $submit[] = $app->id;
            if ($us->manage) {
                $manage[] = $app->id;
            }
        }
    }
    return [$items, $submit, $manage];
}

function handle_edit_form() {
    $user_id = get_int('user_id');
    $user = BoincUser::lookup_id($user_id);
    $usub = BoincUserSubmit::lookup_userid($user_id);
    page_head_select2("Job submission permissions for $user->name");
    form_start('manage_project.php');
    form_input_hidden('action', 'edit_action');
    form_input_hidden('user_id', $user->id);
    [$apps, $submit, $manage] = get_app_lists($user_id);
    form_radio_buttons('Can submit jobs to', 'submit_all',
        [[1, 'All apps'], [0, 'Only selected apps']],
        $usub->submit_all, true
    );
    form_select2_multi(
        '', 'submit_apps', $apps, $submit, "id=submit_apps"
    );
    form_radio_buttons('Can administer', 'manage_all',
        [[1, 'All apps'], [0, 'Only selected apps']],
        $usub->manage_all, true
    );
    form_select2_multi(
        '', 'manage_apps', $apps, $manage, "id=manage_apps"
    );
    form_input_text('Quota', 'quota', $usub->quota);
    form_input_text('Max jobs in progress',
        'max_jobs_in_progress', $usub->max_jobs_in_progress
    );
    form_general('Run jobs only on own computers?',
        $user->seti_id?'yes':'no'
    );
    form_submit('Update');
    form_end();

    // disable the app selector if 'All apps' checked
    //
    echo "
<script>
var select_apps = document.getElementById('submit_apps');
var submit_all_0 = document.getElementById('submit_all_0');
var submit_all_1 = document.getElementById('submit_all_1');
fsubmit = function() {
    select_apps.disabled = submit_all_1.checked;
};
fsubmit();
submit_all_0.onchange = fsubmit;
submit_all_1.onchange = fsubmit;

var manage_apps = document.getElementById('manage_apps');
var manage_all_0 = document.getElementById('manage_all_0');
var manage_all_1 = document.getElementById('manage_all_1');
fmanage = function() {
    manage_apps.disabled = manage_all_1.checked;
};
fmanage();
manage_all_0.onchange = fmanage;
manage_all_1.onchange = fmanage;

</script>
    ";
    page_tail();
}

function handle_edit_action() {
    $user_id = get_int('user_id');
    $us = BoincUserSubmit::lookup_userid($user_id);
    if (!$us) error_page("user not found");
    BoincUserSubmitApp::delete_user($user_id);
    if (get_str('submit_all')) {
        $us->update("submit_all=1");
    } else {
        $us->update("submit_all=0");
    }
    if (get_str('manage_all')) {
        $us->update("manage_all=1");
    } else {
        $us->update("manage_all=0");
    }
    $apps = BoincApp::enum("deprecated=0");
    $submit_apps = get_array('submit_apps');
    $submit_apps = array_map('intval', $submit_apps);
    $manage_apps = get_array('manage_apps');
    $manage_apps = array_map('intval', $manage_apps);
    foreach ($apps as $app) {
        $s = in_array($app->id, $submit_apps);
        $m = in_array($app->id, $manage_apps)?1:0;
        if ($s || $m) {
            BoincUserSubmitApp::insert(
                "(user_id, app_id, manage) values ($user_id, $app->id, $m)"
            );
        }
    }
    $quota = (double) get_str('quota');
    if ($quota != $us->quota) {
        $us->update("quota=$quota");
    }
    $mj = (int) get_str('max_jobs_in_progress');
    if ($mj != $us->max_jobs_in_progress) {
        $us->update("max_jobs_in_progress=$mj");
    }
    header('Location: manage_project.php');
}

function handle_add_form() {
    page_head("Add user");
    echo "
        <form action=manage_project.php>
        <input type=hidden name=action value=add_action>
        User ID: <input name=user_id>
        <br>
        <input class=\"btn btn-success\" type=submit value=OK>
        </form>
    ";
    page_tail();
}

function handle_add_action() {
    $user_id = get_int('user_id');
    $user = BoincUser::lookup_id($user_id);
    if (!$user) error_page("no such user");
    $us = BoincUserSubmit::lookup_userid($user_id);
    if (!$us) {
        if (!BoincUserSubmit::insert("(user_id, quota, logical_start_time, submit_all, manage_all, max_jobs_in_progress) values ($user_id, 1, 0, 0, 0, 10000)")) {
            error_page("Insert failed");
        }
    }
    header("Location: manage_project.php?action=edit_form&user_id=$user_id");
}

$user = get_logged_in_user();
$bus = BoincUserSubmit::lookup_userid($user->id);
if (!$bus) {
    error_page("no access");
}

$action = get_str('action', true);
switch ($action) {
case 'list':
case '':
    handle_list(); break;
case 'add_form':
    handle_add_form(); break;
case 'add_action':
    handle_add_action(); break;
case 'edit_form':
    handle_edit_form(); break;
case 'edit_action':
    handle_edit_action(); break;
default:
    error_page("unknown action");
}

?>
