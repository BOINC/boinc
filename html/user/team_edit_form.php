<?php

require_once("util.inc");
require_once("team.inc");

    db_init();
    $user = get_logged_in_user();

    $teamid = $_GET["teamid"];

    $query = "select * from team where id = $teamid";
    $result = mysql_query($query);
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    require_founder_login($user, $team);
    $team_name = ereg_replace("\"", "'", $team->name);
    $team_name_html = ereg_replace("\"", "'", $team->name_html);
    $team_url = ereg_replace("\"", "'", $team->url);
    $team_description = ereg_replace("\"", "'", $team->description);
    $team_type = $team->type;
    page_head("Edit $team_name");
    team_edit_form($team, "Update team info", "team_edit_action.php");
    page_tail();
?>
