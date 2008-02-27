<?php

require_once("../inc/util.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

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
        $x = "$y<a href=db_action.php?table=user&id=$asgn->target_id>Host $asgn->target_id</a>";
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
