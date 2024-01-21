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

// app-specific management interface

require_once("../inc/submit_util.inc");
require_once("../inc/util.inc");

function main_page($app) {
    page_head("Management functions for $app->name");
    echo "
        <p>
        <a href=manage_app.php?app_id=$app->id&amp;action=app_version_form>Manage app versions</a>
        <p>
        <a href=manage_app.php?app_id=$app->id&amp;action=permissions_form>Manage user permissions</a>
        <p>
        <a href=manage_app.php?app_id=$app->id&amp;action=batches_form>Manage jobs</a>
    ";
    page_tail();
}

function app_version_form($app) {
    page_head("Manage app versions");
    echo "
        <form action=manage_app.php>
        <input type=hidden name=action value=app_version_action>
        <input type=hidden name=app_id value=$app->id>
    ";
    $avs = BoincAppVersion::enum("appid=$app->id");
    start_table();
    table_header("platform", "plan class", "version#", "deprecated");
    foreach ($avs as $av) {
        $platform = BoincPlatform::lookup_id($av->platformid);
        $c = $av->deprecated?"checked":"";
        echo "
            <tr>
            <td>$platform->name</td>
            <td>$av->plan_class</td>
            <td>$av->version_num</td>
            <td><input type=checkbox name=dep_$av->id $c></td>
            </tr>
        ";
    }
    echo "
        <tr>
        <td><br></td>
        <td><br></td>
        <td><br></td>
        <td><input class=\"btn btn-default\" type=submit value=Update></td>
        </tr>
    ";
    end_table();
    echo "<form>\n";
    page_tail();
}

function app_version_action($app) {
    $avs = BoincAppVersion::enum("appid=$app->id");
    foreach ($avs as $av) {
        $x = get_str("dep_$av->id", true);
        if ($x) {
            if (!$av->deprecated) {
                $av->update("deprecated=1");
            }
        } else {
            if ($av->deprecated) {
                $av->update("deprecated=0");
            }
        }
    }
    page_head("Update successful");
    echo "
        <a href=manage_app.php?app_id=$app->id>Return to application management page</a>
    ";
    page_tail();
}

function permissions_form($app) {
    page_head("Manage user permissions for $app->name");
    echo "
        <form action=manage_app.php>
        <input type=hidden name=action value=permissions_action>
        <input type=hidden name=app_id value=$app->id>
    ";
    $busas = BoincUserSubmitApp::enum("app_id=$app->id");
    start_table();
    table_header("User", "Allowed to submit jobs to $app->name");
    foreach ($busas as $busa) {
        $user = BoincUser::lookup_id($busa->user_id);
        echo "
            <tr>
            <td>$user->name (ID: $user->id)</td>
            <td><input type=checkbox name=user_$user->id checked></td>
            </tr>
        ";
    }
    echo "
        <tr>
        <td>Add new user</td>
        <td>User ID: <input name=new_user_id></td>
        </tr>
        <tr>
        <td><br></td>
        <td><input class=\"btn btn-default\" type=submit value=OK></td>
        </tr>
    ";
    end_table();
    echo "<form>\n";
    page_tail();
}

function permissions_action($app) {
    $busas = BoincUserSubmitApp::enum("app_id=$app->id");
    foreach ($busas as $busa) {
        if (!get_str("user_$busa->user_id", true)) {
            BoincUserSubmitApp::delete_user($busa->user_id);
        }
    }
    $userid = get_int("new_user_id", true);
    if ($userid) {
        BoincUserSubmitApp::insert("(user_id, app_id) values ($userid, $app->id)");
    }
    page_head("Update successful");
    echo "
        <a href=manage_app.php?app_id=$app->id>Return to application management page</a>
    ";
    page_tail();
}

function batches_form($app) {
    page_head("Manage jobs for $app->name");
    echo "
        <form action=manage_app.php>
        <input type=hidden name=action value=batches_action>
        <input type=hidden name=app_id value=$app->id>
    ";
    start_table();
    table_header("Batch ID", "Submitter", "Submitted", "State", "# jobs", "Abort?");
    $batches = BoincBatch::enum("app_id=$app->id");
    foreach ($batches as $batch) {
        $user = BoincUser::lookup_id($batch->user_id);
        echo "<tr>
            <td>$batch->id</td>
            <td>$user->name</td>
            <td>".time_str($batch->create_time)."</td>
            <td>".batch_state_string($batch->state)."
            <td>$batch->njobs</td>
            <td><input type=checkbox name=abort_$batch->id></td>
            </tr>
        ";
    }
    echo "<tr>
        <td colspan=5>Abort all jobs for $app->name?</td>
        <td><input type=checkbox name=abort_all></td>
        </tr>
    ";
    echo "<tr>
        <td><br></td>
        <td><br></td>
        <td><br></td>
        <td><input class=\"btn btn-default\" type=submit value=OK></td>
        </tr>
    ";
    end_table();
    page_tail();
}

function batches_action($app) {
    $batches = BoincBatch::enum("app_id=$app->id");
    $abort_all = (get_str("abort_all", true));
    foreach ($batches as $batch) {
        if ($abort_all || get_str("abort_$batch->id", true)) {
            abort_batch($batch);
        }
    }
    page_head("Update successful");
    echo "
        <a href=manage_app.php?app_id=$app->id>Return to application management page</a>
    ";
    page_tail();
}

$user = get_logged_in_user();
$app_id = get_int("app_id");
$app = BoincApp::lookup_id($app_id);
if (!$app) error_page("no such app");
$bus = BoincUserSubmit::lookup_userid($user->id);
if (!$bus) error_page("no access");
if (!$bus->manage_all) {
    $busa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app_id");
    if (!$busa || !$busa->manage) error_page("no access");
}

$action = get_str("action", true);
switch ($action) {
case "":
    main_page($app); break;
case "app_version_form":
    app_version_form($app); break;
case "app_version_action":
    app_version_action($app); break;
case "permissions_form":
    permissions_form($app); break;
case "permissions_action":
    permissions_action($app); break;
case "batches_form":
    batches_form($app); break;
case "batches_action":
    batches_action($app); break;
default:
    error_page("unknown action $action");
}
?>
