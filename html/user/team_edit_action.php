<?php

    require_once("db.inc");
    require_once("util.inc");
    require_once("team.inc");

    db_init();
    $user = get_logged_in_user();
    $teamid = $_POST["teamid"];

    $query = "select * from team where id = $teamid";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    require_founder_login($user, $team);

    $team_url = ereg_replace("\"", "'", $_POST["url"]);
    $x = strstr($team_url, "http://");
    if ($x) {
        $team_url = substr($team_url, 7);
    }
    $team_name = ereg_replace("\"", "'", $_POST["name"]);
    $team_name_html = ereg_replace("\"", "'", $_POST["name_html"]);
    $team_description = ereg_replace("\"", "'", $_POST["description"]);

    $query_team_table = sprintf(
        "update team set name = '%s',
        name_html = '%s',
        url = '%s',
        description = '%s',
        type = %d,
        country='%s'
        where id = %d",
        $team_name,
        $team_name_html,
        $team_url,
        $team_description,
        $_POST["type"],
        $_POST["country"],
        $team->id
    );
    $result = mysql_query($query_team_table);
    if ($result) {
        Header("Location: team_display.php?teamid=$team->id");
    } else {
        page_head("Error");
        echo "Couldn't update team - please try later.\n";
        page_tail();
    }

?>
