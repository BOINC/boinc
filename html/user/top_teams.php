<?php
    require_once("util.inc");
    require_once("user.inc");

function team_table_start() {
    echo VISTABLE."
        <tr><th>Name</th><th>Members</th><th>Average credit</th><th>Total credit</th>
        <th>Founded</th></tr>";
}

function show_team_row($team) {
    echo "<tr>
        <td><a href=team_display.php?id=$team->id>$team->name</a></td>
        <td>$team->nusers</td>
        <td>$team->expavg_credit</td>
        <td>$team->total_credit</td>
        <td>".time_str($team->create_time)."</td>
    <tr>\n";
}

    db_init();

    page_head("Top teams");
    $result = mysql_query("select * from team order by expavg_credit desc, total_credit desc");
    team_table_start();
    while ($team = mysql_fetch_object($result)) {
        show_team_row($team);
    }
    mysql_free_result($result);
    echo "</table>\n<p>\n";
    page_tail();
?>
