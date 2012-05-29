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
//   - create apps

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");

function user_row($u) {
    $user = BoincUser::lookup_id($u->user_id);
    echo "
        <tr>
        <td>$user->name (ID: $user->id)
        <a href=manage_project.php?action=edit_form&user_id=$u->user_id>Edit permissions</a>
        </td>
    ";
    echo "<td>";
    if ($u->submit_all) {
        echo "All applications\n";
    } else {
        $uas = BoincUserSubmitApp::enum("user_id=$u->user_id");
        foreach ($uas as $ua) {
            $app = BoincApp::lookup_id($ua->app_id);
            echo "$app->name ";
        }
    }
    echo "</td>\n";
    echo "<td>$u->quota</td>\n";
    echo "<td>
        </td>
        </tr>
    ";
}

function handle_list() {
    page_head("Project-wide management functions");
    echo "<h3>User permissions and quotas</h3>
        The following users are allowed to submit jobs.
        <p>
    ";
    show_button("manage_project.php?action=add_form",
        "Add user", "Allow a new user to submit jobs"
    );

    $us = BoincUserSubmit::enum("");
    start_table();
    table_header("User", "Can submit jobs for", "Quota");
    foreach ($us as $u) {
        user_row($u);
    }
    end_table();
    page_tail();
}

function handle_edit_form() {
    $user_id = get_int('user_id');
    $user = BoincUser::lookup_id($user_id);
    $usub = BoincUserSubmit::lookup_userid($user_id);
    page_head("Permissions for $user->name");
    echo "
        $user->name is allowed to submit jobs for:
        <p>
        <form action=manage_project.php>
        <input type=hidden name=action value=edit_action>
        <input type=hidden name=user_id value=$user_id>
    ";
    if ($usub->submit_all) {
        $all_checked = "checked";
        $not_all_checked = "";
    } else {
        $all_checked = "";
        $not_all_checked = "checked";
    }
    echo "<input type=radio name=submit_all value=1 $all_checked> All apps
        <br>
        <input type=radio name=submit_all value=0 $not_all_checked> Only selected apps:
    ";
    $apps = BoincApp::enum("deprecated=0");
    foreach ($apps as $app) {
        $us = BoincUserSubmitApp::lookup("user_id=$user_id and app_id=$app->id");
        $checked = $us?"checked":"";
        echo "<br>&nbsp;&nbsp;&nbsp; <input type=checkbox name=app_$app->id $checked> $app->name\n";
    }
    $q = (string) $usub->quota;
    $sav = $usub->create_app_versions?"checked":"";
    $sa = $usub->create_apps?"checked":"";
    echo "
        <p>
        Quota: <input name=quota value=$q>
        This determines how much computing capacity is allocated to $user->name.
        <p>
        <input type=submit value=OK>
        </form>
        <p>
        <a href=manage_project.php>Return to project-wide management functions</a>
    ";
    page_tail();
}

function handle_edit_action() {
    $user_id = get_int('user_id');
    $us = BoincUserSubmit::lookup_userid($user_id);
    if (!$us) error_page("user not found");
    BoincUserSubmitApp::delete_user($user_id);
    $submit_all = get_str('submit_all');
    if ($submit_all) {
        $us->update("submit_all=1");
    } else {
        $us->update("submit_all=0");
        $apps = BoincApp::enum("deprecated=0");
        foreach ($apps as $app) {
            $x = "app_$app->id";
            if (get_str($x, true)) {
                BoincUserSubmitApp::insert("(user_id, app_id) values ($user_id, $app->id)");
            }
        }
    }
    $quota = (double) get_str('quota');
    if ($quota != $us->quota) {
        $us->update("quota=$quota");
    }
    page_head("Update successful");
    echo "<a href=manage_project.php>Return to project-wide management functions</a>";
    page_tail();
}

function handle_add_form() {
    page_head("Add user");
    echo "
        <form action=manage_project.php>
        <input type=hidden name=action value=add_action>
        User ID: <input name=user_id>
        <br>
        <input type=submit value=OK>
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
        if (!BoincUserSubmit::insert("(user_id) values ($user_id)")) {
            error_page("Insert failed");
        }
    }
    header("Location: manage_project.php?action=edit_form&user_id=$user_id");
}

$user = get_logged_in_user();
$bus = BoincUserSubmit::lookup_userid($user->id);
if (!$bus) {
    die("no access");
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
    error_page("unknown action: $action");
}

?>
