<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    $id = $HTTP_POST_VARS["id"];

    $query = "select * from team where id = $id";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    require_founder_login($user, $team);

    $team_url = ereg_replace("\"", "'", $HTTP_POST_VARS["url"]);
    $pos = strpos($team_url, "http://");
    if (!($pos === false)) { // note: three equal signs
      $team_url = substr($team_url, 7);
    }
    $team_name = ereg_replace("\"", "'", $HTTP_POST_VARS["name"]);
    $team_name_html = ereg_replace("\"", "'", $HTTP_POST_VARS["name_html"]);
    $team_description = ereg_replace("\"", "'", $HTTP_POST_VARS["description"]);


        $query_team_table = sprintf(
            "update team set name = '%s',
            name_html = '%s',
            url = '%s',
            description = '%s',
            type = %d
            where id = %d",
            $team_name,
            $team_name_html,
            $team_url,
            $team_description,
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

page_tail();

?>
