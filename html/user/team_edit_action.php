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
        echo "Only a team's founder may edit a team.";
    } else {
        $query_team_table = sprintf(
            "update team set name = '%s',
            name_html = '%s',
            url = '%s',
            description = '%s',
            type = %d
            where id = %d",
            $HTTP_POST_VARS["name"],
            $HTTP_POST_VARS["name_html"],
            $HTTP_POST_VARS["url"],
            $HTTP_POST_VARS["description"],
            $HTTP_POST_VARS["type"],
            $team->id
        );
        $result_team_table = mysql_query($query_team_table);
        if ($result_team_table) {
            page_head("Changes accepted");
            $team_name = $team->name;
            echo "<h2>Changes Accepted</h2>";
            echo "The changes to <a href=team_display.php?id=$team->id>$team_name</a> were accepted and should now be in effect.";
        } else {
            page_head("Error");
            echo "Couldn't edit team - please try later.\n";
        }
    }

page_tail();

?>
