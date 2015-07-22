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

require_once("../inc/util_ops.inc");

function show_assign($asgn) {
    $when = time_str($asgn->create_time);
    switch ($asgn->target_type) {
    case 0:
        $x = "All hosts";
        break;
    case 1:
        $x = "<a href=db_action.php?table=host&id=$asgn->id>Host $asgn->target_id</a>";
        break;
    case 2:
        if ($asgn->multi) {
            $y = "All hosts belonging to ";
        } else {
            $y = "One host belonging to ";
        }
        $x = "$y<a href=db_action.php?table=user&id=$asgn->target_id>User $asgn->target_id</a>";
        break;
    case 3:
        if ($asgn->multi) {
            $y = "All hosts belonging to ";
        } else {
            $y = "One host belonging to ";
        }
        $x = "$y<a href=db_action.php?table=team&id=$asgn->target_id>Team $asgn->target_id</a>";
        break;
    }
    echo "<tr>
        <td>$asgn->id (created $when)</td>
        <td>$x</td>
        <td><a href=db_action.php?table=workunit&id=$asgn->workunitid>$asgn->workunitid</a></td>
        <td><a href=db_action.php?table=result&id=$asgn->resultid>$asgn->resultid</a></td>
        </tr>
    ";
}

function show_assigns() {
    admin_page_head("Assignments");
    $asgns = BoincAssignment::enum();
    if (count($asgns)) {
        start_table();
        table_header("Assignment ID/time", "target", "workunit", "result");
        foreach ($asgns as $asgn) {
            show_assign($asgn);
        }
        end_table();
    } else {
        echo "No assignments";
    }
    admin_page_tail();
}

$action = get_str('action', true);
switch ($action) {
default:
    show_assigns();
}

?>
