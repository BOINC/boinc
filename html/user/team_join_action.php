<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    db_init();
    $user = get_logged_in_user();

    $teamid = $_POST["teamid"];
    $result = mysql_query("select * from team where id = $teamid");
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
    require_team($team);
    if ($user->teamid == $team->id) {
        page_head("Unable to add $user->name");
        echo "You are already a member of $team->name.";
    } else {
        if ($user->teamid != 0) {
            $query_team_other = sprintf(
                "select * from team where id = %d",
                $user->teamid
            );
            $result_team_other = mysql_query($query_team_other);
            $first_team = mysql_fetch_object($result_team_other);
            $first_nusers = $first_team->nusers;
            $first_new_nusers = $first_nusers - 1;
            $query_team_table_other = sprintf(
                "update team set nusers = %d where id = %d",
                $first_new_nusers,
                $first_team->id
            );
            $result_team_table_other = mysql_query($query_team_table_other);
        }
        $query_user_table = sprintf(
            "update user set teamid = %d where id = %d",
            $team->id,
            $user->id
        );
        $result_user_table = mysql_query($query_user_table);
	$nusers = $team->nusers;
        $new_nusers = $nusers + 1;
        $query_team_table = sprintf(
            "update team set nusers = %d where id = %d",
            $new_nusers,
            $team->id
        );
        $result_team_table = mysql_query($query_team_table);
        if ($result_user_table && $result_team_table) {
            page_head("Joined $team_name");
            echo "<h2>Joined team</h2>";
            echo "You have joined
                <a href=team_display.php?teamid=$team->id>$team_name</a>.
            ";
        } else {
            page_head("Error");
            echo "Couldn't join team - please try later.\n";
        }
    }

page_tail();

?>
