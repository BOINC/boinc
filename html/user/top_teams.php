<?php
    require_once("util.inc");
    require_once("user.inc");

function team_table_start() {
    echo TABLE."
        <tr><th>Name</th><th>Average credit</th><th>Total credit</th>
        <th>Founded</th></tr>";
}

function show_team_row($team) {
    echo "<tr>
        <td>$team->name</td>
        <td>$team->expavg_credit</td>
        <td>$team->total_credit</td>
        <td>".time_str($team->create_time)."</td>
    <tr>\n";
}

    db_init();

    // update team stats; eventually this must be moved
    // to a separate program
    //
    $result = mysql_query("select * from team");
    while ($team = mysql_fetch_object($result)) {
        $query = "select sum(total_credit) from user where teamid = $team->id";
        $result = mysql_query($query);
        $total_credit_sum = mysql_result($result, 0);

        $query = "select sum(expavg_credit) from user where teamid = $team->id";
        $result = mysql_query($query);
        $expavg_credit_sum = mysql_result($result, 0);

        $query = "select count(*) from user where teamid = $team->id";
        $result = mysql_query($query);
        $nmembers = mysql_result($result, 0);

        $total_credit = $total_credit_sum/$nmembers;
        $expavg_credit = $expavg_credit_sum/$nmembers;
        $query = "update team set total_credit=$total_credit, expavg_credit=$expavg_credit where id=$team->id";
        $result = mysql_query($query);
    }
    mysql_free_result($result);

    page_head("Top teams");
    $result = mysql_query("select * from team order by expavg_credit desc");
    team_table_start();
    while ($team = mysql_fetch_object($result)) {
        show_team_row($team);
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
