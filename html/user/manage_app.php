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

// app management interface
//      - deprecate app versions

require_once("../inc/submit_util.inc");
require_once("../inc/util.inc");

function app_version_form($app) {
    page_head("Manage versions of $app->name");
    echo "
        <form action=manage_app.php>
        <input type=hidden name=action value=app_version_action>
        <input type=hidden name=app_id value=$app->id>
    ";
    $avs = BoincAppVersion::enum("appid=$app->id");
    start_table('table-striped');
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
        <td><input class=\"btn\" type=submit value=Update></td>
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
        <a href=submit.php>Return to job submission page</a>
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
case "app_version_form":
    app_version_form($app); break;
case "app_version_action":
    app_version_action($app); break;
default:
    error_page("unknown action");
}
?>
