<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);

    $query = sprintf(
        "select * from team where id = %d",
        $HTTP_POST_VARS["id"]
    );
    $result = mysql_query($query);
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
    if ($user->id != $team->userid) {
        page_head("Permission denied");
        echo "Only a team's founder may remove members from a team.";
    } else {
        $nmembers = 0;
        $unable_to_remove = FALSE;
        $user_table_error = FALSE;
        for ($i=0; $i<$HTTP_POST_VARS["ninactive_users"]; $i++) {
            if ($HTTP_POST_VARS["remove_$i"] != 0) {
                $query_user_teamid = sprintf(
                    "select * from user where id = %d",
                    $HTTP_POST_VARS["remove_$i"]
                );
                $result_user_teamid = mysql_query($query_user_teamid);
                $curr_user = mysql_fetch_object($result_user_teamid);
                if ($curr_user->teamid != $team->id) {
                    echo "$curr_user->name is not a member of <a href=team_display.php?id=$id>$team_name</a>";
                    $unable_to_remove = TRUE;
                } else {
                    $query_user_table = sprintf(
                        "update user set teamid = %d where id = %d",
                        0,
                       $HTTP_POST_VARS["remove_$i"]
                    );
                    $nmembers++;
                    $result_user_table = mysql_query($query_user_table);
                 }
             }
        }
        $nusers = $team->nusers;
        $new_nusers = $nusers - $nmembers;
        $query_team_table = sprintf(
            "update team set nusers = %d where id = %d",
            $new_nusers,
            $team->id
        );
        $result_team_table = mysql_query($query_team_table);
        if ($result_team_table && $result_user_table) {
            $team_name = $team->name;
            page_head("Removed Team Members");
            echo "<h2>Removed Team Members</h2>";
            echo "Removing team members from <a href=team_display.php?id=$team->id>$team_name</a>:<p> ";
            for ($i=0; $i<$HTTP_POST_VARS["ninactive_users"]; $i++) {
                if ($HTTP_POST_VARS["remove_$i"] != 0) {
                    $query_user_table = sprintf(
                        "select * from user where id = %d",
                        $HTTP_POST_VARS["remove_$i"]
                    );
                    $result_user_table = mysql_query($query_user_table);
                    $user = mysql_fetch_object($result_user_table);
                    mysql_free_result($result_user_table);
                    echo "$user->name has been removed<br>";
                }
            }
        } else {
            page_head("Error");
            echo "Couldn't remove users - please try later.\n";
        }
    }

page_tail();

?>
