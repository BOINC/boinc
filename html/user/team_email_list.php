<?php

require_once("util.inc");
require_once("team.inc");
require_once("db.inc");

db_init();

$user = get_user_from_cookie();

$query = sprintf(
    "select * from team where id=%d",
    $HTTP_GET_VARS["id"]
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
}
if (!$team) {
    page_head("Unable to display team members' email addresses");
    echo ("We are unable to display the email addresses of the members of that team");
    page_tail();
} else if ($user->id != $team->userid) {
    page_head("Permission denied");
    echo "Only a team's founder may view a team's email list.";
} else {
    page_head("$team->name Email List");
    echo "<p>";
    echo "<table border=0 width=580>";
    echo "<tr bgcolor=#708090><td colspan=2><font size=+1>";
    echo "<b>Team Email List:   </b></font></td></tr>";
    echo "<tr><td width=25% valign=top align=left><b>Name:</b></td>";
    echo "<td><b>Email Address:</b></td></tr>\n";
    $query = sprintf(
        "select * from user where teamid=%d",
        $team->id
    );
    $result = mysql_query($query);
    if ($result) {
        for ($i = 0; $i < $team->nusers; $i++) {
            $user_team = mysql_fetch_object($result);
            echo "<tr><td>$user_team->name</td>";
            echo "<td><a href=mailto:$user_team->email_addr>$user_team->email_addr</a></td></tr>";
        } 
    }
    echo "</table>";
}
page_tail();

?>
