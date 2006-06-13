<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/team.inc");

    db_init();
    $user = get_logged_in_user();
    $teamid = $_POST["id"];
    $team = lookup_team($teamid);
    if ($user->teamid == $team->id) {
        user_quit_team($user);
        page_head("Quit $team->name");
        echo "You have been removed from <a href=team_display.php?teamid=$team->id>$team->name</a>";
    } else {
        page_head("Unable to quit team");
        echo "Team doesn't exist, or you don't belong to it.\n";
    }

page_tail();

?>
