<?php

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

// show the results of ops/analyze_coproc_log.php

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
        echo "Computer ID<br><span class=note>click for details</span>";
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

function show_row($x, $y, $mode, $i) {
    $class = $i%2?"row0":"row1";
    echo "<tr class=$class><td>";
    switch ($mode) {
    case 'host':
        echo "<a href=show_host_detail.php?hostid=$x>$x</a>";
        break;
    case 'user':
        $user = BoincUser::lookup_id($x);
        echo "<a href=show_user.php?userid=$x>$user->name</a>";
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

start_table();
header_row($mode);
$i = 0;
foreach ($array as $x=>$y) {
    show_row($x, $y, $mode, $i);
    $i++;
}
end_table();

page_tail();

?>
