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

// top-level management page;
// shows links to the various functions available to the user.
// If the only option is managing a particular app,
// redirect to that page

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");

$user = get_logged_in_user();

$bus = BoincUserSubmit::lookup_userid($user->id);
if (!$bus) die("no access");

if ($bus->manage_all) {
    page_head("Management functions");
    echo "
        <a href=manage_project.php>Project-wide management</a>
    ";
    $apps = BoincApp::enum(null);
    echo "
        <p>Application-specific management:
        <ul>
    ";
    foreach ($apps as $app) {
        echo "
            <li><a href=manage_app.php?app_id=$app->id>$app->name</a>
        ";
    }
    echo "</ul>\n";
    page_tail();
    exit;
}

$apps = BoincUserSubmit::enum("user_id=$user->id and manage<>1");
switch (count($apps)) {
case 0:
    error_page("Nothing to manage");
case 1:
    $app = $apps[0];
    Header("Location: manage_app.php?app_id=$app->id");
    exit;
default:
    page_head("Management functions");
    foreach ($apps as $app) {
        echo "
            <p><a href=manage_app.php?app_id=$app->id>Manage $app->name</a>
        ";
    }
    page_tail();
}

?>
