<?php {

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/team.inc");

    db_init();
    $user = get_logged_in_user();

    $teamid = $_POST["id"];
    $team = lookup_team($teamid);
    if (!team) {
        error_page("No such team");
    }
    require_founder_login($user, $team);

    page_head("Removing users from $team->name");
    $ndel = 0;
    for ($i=0; $i<$_POST["ninactive_users"]; $i++) {
        if ($_POST["remove_$i"] != 0) {
            $userid = $_POST["remove_$i"];
            $user = lookup_user_id($userid);
            if ($user->teamid != $team->id) {
                echo "<br>$user->name is not a member of $team->name";
            } else {
                $query = "update user set teamid=0 where id=$userid";
                $result_user_table = mysql_query($query);
                echo "<br>$user->name has been removed";
                $ndel++;
             }
         }
    }
    if ($ndel) {
        team_update_nusers($team);
    }

    page_tail();

} ?>
