<?php
    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    db_init();
    $user = get_logged_in_user();

    $query = sprintf(
        "select * from team where id = %d",
        $_POST["id"]
    );
    $result = mysql_query($query);
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
    if ($user->teamid == $team->id) {
        $query_user_table = sprintf(
            "update user set teamid = %d where id = %d",
            0,
            $user->id
        );
        $result_user_table = mysql_query($query_user_table);
	$nusers = $team->nusers;
        $new_nusers = $nusers - 1;
        $query_team_table = sprintf(
            "update team set nusers = %d where id = %d",
            $new_nusers,
            $team->id
        );
        $result_team_table = mysql_query($query_team_table);
        if ($result_user_table && $result_team_table) {
            $team_name = $team->name;
            page_head("Quit $team_name");
            echo "<h2>Removed from team</h2>";
            echo "You have been removed from <a href=team_display.php?teamid=$team->id>$team_name</a>";
        } else {
            page_head("Error");
            echo "Couldn't quit team - please try later.\n";
        }
    } else {
        page_head("Unable to remove $user->name");
        echo "$user->name is not a member of $team_name.\n";
    }

page_tail();

?>
