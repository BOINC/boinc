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
        $success = user_join_team($team,$user);
        if ($success == true) {
            page_head("Joined $team->name");
            echo "<h2>Joined team</h2>";
            echo "You have joined
                <a href=team_display.php?teamid=$team->id>$team->name</a>.
            ";
        } else {
            page_head("Error");
            echo "Couldn't join team - please try later.\n";
        }
    }

page_tail();

?>
