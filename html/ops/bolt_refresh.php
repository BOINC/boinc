<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// send reminder emails for Bolt refresh.
// Run this script at most once a day
// (to avoid multiple emails)

$cli_only = true;
require_once("../inc/bolt_db.inc");
require_once("../inc/util_ops.inc");

function notify_user($user) {
    $body = "
You are due for refresh on the following units from 
";
    foreach($user->refresh as $r) {
        echo "
    }
}

function notify_users() {
    $now = time();
    $rs = BoltRefresh::enum("due_time < $now");
    $users = array();

    foreach($rs as $r) {
        $view = BoltView::lookup_id($r->view_id);
        $user_id = $view->user_id;
        if (!key_exists($user_id)) {
            $user = BoincUser::lookup_id($user_id);
            BoltUser::lookup($user);
            $user->refresh = array();
            $users[$user_id] = $user;
        }
        $users[$user_id]->refresh[] = $r;
    }

    foreach ($users as $user) {
        notify_user($user);
    }
}

?>
