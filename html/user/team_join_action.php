<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    require_login($user);

    $query = sprintf(
        "select * from team where id = %d",
        $HTTP_POST_VARS["id"]
    );
    $result = mysql_query($query);
    $team = mysql_fetch_object($result);
    $team_name = $team->name;
    mysql_free_result($result);
    if ($user->teamid == $team->id) {
        page_head("Unable to add $user->name");
        echo "$user->name is already a member of $team_name.";
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
            page_head("Added to $team_name");
            echo "<h2>Added to team</h2>";
            echo "You have been added to <a href=team_display.php?id=$team->id>$team_name</a>.<br>";
            echo "If you were previously a part of a team you are no longer a member of it. ";
            echo "You may only be part of one team at a time.<p>";
        } else {
            page_head("Error");
            echo "Couldn't join team - please try later.\n";
        }
    }

page_tail();

?>
