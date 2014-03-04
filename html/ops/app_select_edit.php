#!/usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// If you have user app selection enabled
// (APP_SELECT_PREFS in html/project/project_specific_prefs.inc)
// and you add a new app, then initially it will be excluded
// for all users who have selected apps.
// This is probably not what you want.
//
// This script adds an app to all users' app-selection project preferences.
// usage: app_select_edit.php app_id
//
// TEST THIS (SEE BOTTOM OF FILE) BEFORE YOU RUN IT

// Implementation notes: structure of prefs is
//
// <project_preferences>
//   <project_specific>
//     <apps_selected>
//       <app_id>N</app_id>
//     </apps_selected>
//   </project_specific>
//   <venue>
//     <project_specific>
//       <apps_selected>
//         <app_id>N</app_id>
//       </apps_selected>
//     </project_specific>
//   </venue>
// </project_preferences>
//
// - we need to add the app to both main prefs and venues
// - the enclosing <apps_selected> may be missing (old server code)

require_once("../inc/boinc_db.inc");

define("VERBOSE", true);

// process a <project_specific> element;
// add the app to the simpleXML object
//
function do_pref_set(&$psp, $app_id) {
    if (!$psp) {
        if (VERBOSE) echo "no prefs\n";
        return;
    }
    if (empty($psp->apps_selected)) {
        $x = $psp;
    } else {
        $x = $psp->apps_selected;
    }
    $appids = $x->app_id;
    if (count($appids) == 0) {
        if (VERBOSE) echo "list empty\n";
        return;
    }
    foreach ($appids as $i) {
        if ((int)$i == $app_id) {
            if (VERBOSE) echo "already in list\n";
            return;
        }
    }
    $x->addChild("app_id", $app_id);
}

// return updated project prefs, or null if error
//
function get_new_prefs($user, $app_id) {
    if (!$user->project_prefs) {
        if (VERBOSE) echo "no project prefs\n";
        return null;
    }
    $prefs = @simplexml_load_string($user->project_prefs);
    if (!$prefs) {
        if (VERBOSE) echo "parse error\n";
        if (VERBOSE) echo $user->project_prefs;
        return null;
    }
    do_pref_set($prefs->project_specific, $app_id);
    foreach ($prefs->venue as $v) {
        do_pref_set($v->project_specific, $app_id);
    }
    $dom = new DOMDocument('1.0');
    $dom->preserveWhiteSpace = false;
    $dom->formatOutput = true;
    $dom->loadXML($prefs->asXML());
    return $dom->saveXML($dom->documentElement) . "\n";
}

function update_user($user, $app_id) {
    if (VERBOSE) echo "processing user $user->id\n";
    $p = get_new_prefs($user, $app_id);
    if ($p) {
        $p = BoincDb::escape_string($p);
        $user->update("project_prefs='$p'");
        if (VERBOSE) echo "updated user $user->id\n";
    }
}

function update_users($app_id) {
    $n = 0;
    $maxid = BoincUser::max("id");
    while ($n <= $maxid) {
        $m = $n + 1000;
        $users = BoincUser::enum("id>=$n and id<$m");
        foreach ($users as $user) {
            update_user($user, $app_id);
        }
        $n = $m;
    }
}

if ($argc != 2) {
    die("usage: app_select_edit.php app_id\n");
}
$app_id = $argv[1];
if (!BoincApp::lookup_id($app_id)) {
    die("No such app: $app_id\n");
}

// change comments below for testing

//echo get_new_prefs(BoincUser::lookup_id(1), $app_id);
    // show the new project prefs for a user, but don't update DB

//update_user(BoincUser::lookup_id(1), $app_id);
    // update DB for a particular user

update_users($app_id);
    // update DB for all users

?>
