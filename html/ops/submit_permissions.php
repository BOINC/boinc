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

// Interface for controlling user permissions to submit jobs

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/util_ops.inc");

function user_row($u) {
    $user = BoincUser::lookup_id($u->user_id);
    echo "
        <tr>
        <td>$user->name<br>$user->email_addr</td>
    ";
    echo "<td>";
    if ($u->all_apps) {
        echo "All\n";
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
        <a href=submit_permissions.php?action=edit_form&user_id=$u->user_id>Edit</a>
        </td>
        </tr>
    ";
}

function handle_list() {
    admin_page_head("User job submission permissions");
    $us = BoincUserSubmit::enum("");
    start_table();
    table_header("User", "Applications", "Quota", "");
    foreach ($us as $u) {
        user_row($u);
    }
    end_table();
    show_button("submit_permissions.php?action=add_form", "Add user", "Add user");
    admin_page_tail();
}

function handle_edit_form() {
    $user_id = get_int('user_id');
    $user = BoincUser::lookup_id($user_id);
    $usub = BoincUserSubmit::lookup_userid($user_id);
    admin_page_head("Set permissions for $user->name");
    echo "
        Can submit jobs for:
        <form action=submit_permissions.php>
        <input type=hidden name=action value=edit_action>
        <input type=hidden name=user_id value=$user_id>
    ";
    if ($usub->all_apps) {
        $all_checked = "checked";
        $not_all_checked = "";
    } else {
        $all_checked = "";
        $not_all_checked = "checked";
    }
    echo "<input type=radio name=all_apps value=1 $all_checked> All apps
        <br>
        <input type=radio name=all_apps value=0 $not_all_checked> Only selected apps:
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
        <input type=checkbox name=create_app_versions $sav> Can create new versions of the above apps
        <p>
        <input type=checkbox name=create_apps $sa> Can create new apps
        <p>
        Quota: <input name=quota value=$q>
        <p>
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
}

function handle_edit_action() {
    $user_id = get_int('user_id');
    $us = BoincUserSubmit::lookup_userid($user_id);
    if (!$us) admin_error_page("user not found");
    BoincUserSubmitApp::delete_user($user_id);
    $all_apps = get_str('all_apps');
    if ($all_apps) {
        $us->update("all_apps=1");
    } else {
        $us->update("all_apps=0");
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
    $x = get_str('create_apps', true)?1:0;
    $us->update("create_apps=$x");
    $x = get_str('create_app_versions', true)?1:0;
    $us->update("create_app_versions=$x");

    admin_page_head("User permissions updated");
    admin_page_tail();
}

function handle_add_form() {
    admin_page_head("Add user");
    echo "
        <form action=submit_permissions.php>
        <input type=hidden name=action value=add_action>
        User ID: <input name=user_id>
        <br>
        <input type=submit value=OK>
        </form>
    ";
    admin_page_tail();
}

function handle_add_action() {
    $user_id = get_int('user_id');
    $user = BoincUser::lookup_id($user_id);
    if (!$user) admin_error_page("no such user");
    $us = BoincUserSubmit::lookup_userid($user_id);
    if (!$us) {
        if (!BoincUserSubmit::insert("(user_id) values ($user_id)")) {
            admin_error_page("Insert failed");
        }
    }
    header("Location: submit_permissions.php?action=edit_form&user_id=$user_id");
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
    admin_error_page("unknown action: $action");
}

?>
