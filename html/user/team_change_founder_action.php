<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user();

$teamid = $_POST["teamid"];
$team = lookup_team($teamid);
if (!team) {
    error_page("No such team");
}
require_founder_login($user, $team);

page_head("Changing founder of $team->name");
$n = $_POST["navailable_users"];
for ($i=0; $i<$n; $i++) {
    if ($_POST["change_$i"] != 0) {
        $userid = $_POST["change_$i"];
        $user = lookup_user_id($userid);
        if ($user->teamid != $team->id) {
            echo "<br>$user->name is not a member of $team->name";
        } else {
            $query = "update team set userid=$userid where id=$team->id";
            $result_user_table = mysql_query($query);
            echo "<br>$user->name is now Founder of $team->name";
        }
    }
}
page_tail();

?>
