<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("team.inc");

    db_init();
    $user = get_logged_in_user();
    $teamid = $_POST["id"]

    $team = lookup_team($teamid);
    if ($user->teamid == $team->id) {
        $query_user_table = "update user set teamid = 0 where id = $user->id";
        $result_user_table = mysql_query($query_user_table);
        $query_team_table = "update team set nusers=nusers-1 where id=$team->id";
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
