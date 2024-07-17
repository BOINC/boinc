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

// show the results of ops/analyze_coproc_log.php

// DEPRECATED

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

check_get_args(array("mode"));

function filename($mode) {
    switch ($mode) {
    case 'host': return "cuda_hosts.dat";
    case 'user': return "cuda_users.dat";
    case 'team': return "cuda_teams.dat";
    case 'model': return "cuda_models.dat";
    case 'day': return "cuda_days.dat";
    }
}

function title($mode) {
    switch ($mode) {
    case 'host': return "Top CUDA hosts";
    case 'user': return "Top CUDA users";
    case 'team': return "Top CUDA teams";
    case 'model': return "Top CUDA models";
    case 'day': return "Daily CUDA credit";
    }
}

function header_row($mode) {
    echo "<tr><th>";
    switch ($mode) {
    case 'host':
        echo "Computer ID<br><p class=\"text-muted\">click for details</p>";
        break;
    case 'user':
        echo "User";
        break;
    case 'team':
        echo "Team";
        break;
    case 'model':
        echo "Model";
        break;
    case 'day':
        echo "Date";
        break;
    }
    echo "</th><th>CUDA Credit</th><th>Number of CUDA jobs</th></tr>\n";
}

function show_row($x, $y, $mode) {
    echo "<tr><td>";
    switch ($mode) {
    case 'host':
        echo "<a href=show_host_detail.php?hostid=$x>$x</a>";
        break;
    case 'user':
        $user = BoincUser::lookup_id($x);
        echo sprintf(
            '<a href=%s?userid=%d>%s</a>',
            SHOW_USER_PAGE, $x, $user->name
        );
        break;
    case 'team':
        $team = BoincTeam::lookup_id($x);
        if ($team) {
            echo "<a href=team_display.php?teamid=$x>$team->name</a>";
        } else {
            echo "(no team)";
        }
        break;
    case 'model':
        echo $x;
        break;
    case 'day':
        echo $x;
        break;
    }
    echo "</td><td align=right>".format_credit_large($y->credit),"</td><td align=right>$y->nresults</td></tr>\n";
}

$mode = get_str('mode', true);
if (!$mode) {
    page_head("Show GPU info");
    echo "
        <ul>
        <li> <a href=show_coproc.php?mode=host>Hosts</a>
        <li> <a href=show_coproc.php?mode=user>Users</a>
        <li> <a href=show_coproc.php?mode=team>Teams</a>
        <li> <a href=show_coproc.php?mode=model>GPU models</a>
        <li> <a href=show_coproc.php?mode=day>Day</a>
        </ul>
    ";
    page_tail();
    exit;
}

$fname = "../ops/".filename($mode);
$data = file_get_contents($fname);
$array = unserialize($data);

page_head(title($mode));

start_table('table-striped');
header_row($mode);
foreach ($array as $x=>$y) {
    show_row($x, $y, $mode);
}
end_table();

page_tail();

?>
