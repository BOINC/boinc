<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    db_init();

    $user = get_logged_in_user();
    if (!strlen($HTTP_POST_VARS["name"])) {
        page_head("Error");
        echo "You must specify a name for your team.";
        exit();
    }
    $query = sprintf(
        "insert into team (userid, create_time, name, name_lc, url, type, name_html, description, country, nusers) values(%d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d)",
        $user->id,
        time(),
        $HTTP_POST_VARS["name"],
        strtolower($HTTP_POST_VARS["name"]),
        $HTTP_POST_VARS["url"],
        $HTTP_POST_VARS["type"],
        $HTTP_POST_VARS["name_html"],
        $HTTP_POST_VARS["description"],
        $HTTP_POST_VARS["country"],
        1
    );

    // TODO: the following logic is very confused

    $result = mysql_query($query);
    if ($result) {
        $query_team = sprintf(
            "select * from team where name = '%s'",
            $HTTP_POST_VARS["name"]
        );
        $result_team = mysql_query($query_team);
        $team = mysql_fetch_object($result_team);
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
    }
    if ($result && $result_user_table) {
        display_team_page($team);
    } else {
        page_head("Error");
        echo "Couldn't create team - please try later.<br>\n";
        echo "You may need to try a different team name.\n";
        page_tail();
        }
    }
?>
