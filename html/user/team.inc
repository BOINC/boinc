<?php

require_once("util.inc");
require_once("db.inc");

function print_teams_display() {

page_head("Teams");
echo "<h2>BOINC Teams</h2>";
echo "<p>";
echo "BOINC users can form &quot;teams&quot; - ";
echo "for example, students in a school, employees of a company. ";
echo "<br>";
echo "Teams can compete to analyze the most data, ";
echo "or they can meet and discuss BOINC projects via the web.";
echo "<br>";
echo "<p>";
echo "<a href=team_create_form.php>Create a new team</a>";
echo "<p>";
echo "<form method=post action=team_lookup.php>";
echo "Search for a team named: ";
echo "<input name=team_name>";
echo "<input type=submit name=search value=\"Search\">";
echo "</form>";
}

function display_team_page($team) {

page_head("$team->name");
if ($team->name_html != null) {
    echo "<p>";
    echo "$team->name_html";
}
echo "<p>";
echo "<a href=team_join_form.php?id=$team->id><b>Join</b></a> | ";
echo "<a href=team_quit_form.php?id=$team->id><b>Quit</b></a> | ";
echo "<a href=team_edit_form.php?id=$team->id><b>Edit*</b></a> | ";
echo "<a href=team_remove_inactive_form.php?id=$team->id><b>Remove Inactive Members*</b></a> | ";
echo "<a href=team_disband_form.php?id=$team->id><b>Disband Team*</b></a> | ";
echo "<a href=team_email_list.php?id=$team->id><b>View Team Emails*</b></a>";
echo "<br><font size=2>* Team founder only</font>";

echo "<p><b>Note: </b>Ocasionally the daily scripts that create this file fail ";
echo "to generate each page correctly.  If you think this has happened, then the ";
echo "founder of this team can regenerate this file at any time by doing the following:";
echo "<ul>";
echo "<li>Click on &quot;Edit&quot;.";
echo "<li>Click on the &quot;Edit Team&quot; button at the bottom of that page.";
echo "</ul>";
echo "<table border=1>";
if (strlen($team->description)) {
    echo "<tr><td>Description</td><td>$team->description</td></tr>";
}
if (strlen($team->url)) {;
    echo "<tr><td>Web site</td><td> <a href=http://$team->url>click here</a></td></tr>";
}
echo "<tr><td>Members</td><td> $team->nusers</td></tr>";

$query = sprintf(
    "select * from user where teamid = %d",
    $team->id
    );
$result = mysql_query($query);
$total_credit = 0;
for ($i = 0; $i < $team->nusers; $i++) {
    $user = mysql_fetch_object($result);
    $total_credit = $total_credit + $user->total_credit;
}

echo "<tr><td>Total Credit</td><td> $total_credit</td></tr>";
echo "<tr><td>Founder</td><td>";
$query = sprintf(
    "select * from user where id = %d",
    $team->userid
);
$result = mysql_query($query);
$user = mysql_fetch_object($result);
echo "$user->name</td></tr>";
echo "</table><p>Team Members:</p>";
echo "<table border=\"1\" cellspacing=0 cellpadding=2><tr>";
echo "<th>Name</th>";
echo "<th>Total<br>Credit</th>";
echo "<th>Average CPU time<br>per work unit</th>";
echo "<th>Country</th>";
echo "</tr>";

$query = sprintf(
    "select * from user where teamid = %d",
    $team->id
);
$result = mysql_query($query);

for ($i = 0; $i < $team->nusers; $i++) {
    $user = mysql_fetch_object($result);
    $j = $i+1;
    echo "<tr><td>$j)$user->name";
    echo "<td align=right>$user->total_credit</td>";
    echo "<td align=right>$user->expavg_time</td>";
    echo "<td align=right>$user->country</td>";
    echo "</tr>";
}
echo "</table>";
}


function print_team_create_form() {

page_head("Create Team");
echo "<h2>Create a BOINC Team</h2>";
echo "<p>";
echo "Use this form to create a team.  ";
echo "You'll become the first member of the team.  ";
echo "Then tell your friends/co-workers/etc. about the team.  ";
echo "They can join it using the BOINC &quot;Teams&quot; page.  ";
echo "<p>";
echo "<form method=post action=team_create_action.php>";
echo "<table>";
echo "<tr>";
echo "<td>Team name (plain-text version):<br><br>&nbsp;</td>";
echo "<td><input name=name type=text size=60>";
echo "<br><font size=2>This name will be printed as-is in the &quot;Top 100&quot; listings and is the name";
echo "<br>you should use when searching for your Team.";
echo "</td>";
echo "</tr><tr>";
echo "<td>Team name (HTML version):<br><br>&nbsp;</td>";
echo "<td><input name=name_html size=60>";
echo "<br><font size=2>This name will be printed as HTML source, so you can include any HTML";
echo "<br>code that you want. This will only be displayed in your teams' page.";
echo "<br>If you don't know HTML, just leave this box blank.";
echo "</td>";
echo "</tr><tr>";
echo "<td>URL of team web page, if any:<br><font size=2>(without &quot;http://&quot;)</td>";
echo "<td><input name=url size=60></td>";
echo "</tr><tr>";
echo "<td valign=top>Description of team:</td>";
echo "<td><textarea name=description cols=60 rows=10></textarea></td>";
echo "</tr><tr>";
echo "<td valign=top>Type of team:</td>";
echo "<td>";
echo "<input type=radio name=type value=4 checked>Club";
echo "<br>";
echo "<input type=radio name=type value=1>Small Company (< 50 employees)";
echo "<br>";
echo "<input type=radio name=type value=2>Medium Company (50-1000 employees)";
echo "<br>";
echo "<input type=radio name=type value=3>Large Company (> 1000 employees)";
echo "<br>";
echo "<input type=radio name=type value=5>Primary School";
echo "<br>";
echo "<input type=radio name=type value=6>Secondary School";
echo "<br>";
echo "<input type=radio name=type value=8>Junior College";
echo "<br>";
echo "<input type=radio name=type value=7>University or Department";
echo "<br>";
echo "<input type=radio name=type value=9>Government Agency";
echo "</td>";
echo "</td>";
echo "</tr></table>";
echo "<input type=submit name=new value=\"Create Team\">";
echo "</form>";
}

function print_team_join_form($id) {

$query = sprintf(
    "select * from team where id = %d",
    $id
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
page_head("Join $team_name");
echo "<h2>Join $team_name</h2>";
echo "<p><b>Please note before joining or switching to a new team:</b>";
echo "<ul>";
echo "<li>If this is the first team you have ever joined ";
echo "then all of your current credit will be given to this ";
echo "team, in addition to all new credit you receive.";
echo "<li>If you are currently a member of a team ";
echo "and are switching to another team, the new team will receive ";
echo "all of your credits, the old teams total will be decreased by ";
echo "your credit amount.";
echo "<li>Joining a team does not affect your personal credit ";
echo "statistics in any way.";
echo "</ul>";
echo "</p>";
echo "<hr>";
echo "<form method=post action=team_join_action.php>";
echo "<input type=hidden name=id value=$team_id>";
echo "<input type=submit value=\"Join Team\">";
echo "</form>";
}

function print_team_quit_form($id) {

$query = sprintf(
    "select * from team where id = %d",
    $id
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
page_head("Quit $team_name");
echo "<h2>Quit $team_name</h2>";
echo "<p><b>Please note before quitting a team:</b>";
echo "<ul>";
echo "<li>By quitting a team you remove your name from the team listing.  ";
echo "Your credit contribution to the team is also removed";
echo "<li>Even if you quit a team, you may rejoin later, ";
echo "or join any other team you desire ";
echo "<li>Quitting a team does not affect your personal credit ";
echo "statistics in any way.";
echo "</ul>";
echo "</p>";
echo "<hr>";
echo "<form method=post action=team_quit_action.php>";
echo "<input type=hidden name=id value=$team_id>";
echo "<input type=submit value=\"Quit Team\">";
echo "</form>";
}

function print_team_edit_form($id) {

$query = sprintf(
    "select * from team where id = %d",
    $id
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
$team_name_html = $team->name_html;
$team_url = $team->url;
$team_description = $team->description;
$team_type = $team->type;
page_head("Edit $team_name");
echo "<h2>Edit $team_name</h2>";
echo "<p><b>Please note before editing a team:</b>";
echo "<ul>";
echo "<li>Only the founder can edit a team";
echo "</ul>";
echo "</p>";
echo "<hr>";
echo "<form method=post action=team_edit_action.php>";
echo "<input type=hidden name=id value=$team_id>";
echo "<table><tr>";
echo "<td>Team name (plain-text version):<br><br>&nbsp;</td>";
echo "<td><input name=name size=60 value=$team_name>";
echo "<br><font size=2>This name will be print as-is";
echo "<br>and is the name you should use when searching for your team.";
echo "</td></tr></tr>";
echo "<td>Team name (HTML version):<br><br>&nbsp;</td>";
echo "<td><input name=name_html size=60 value='$team_name_html'>";
echo "<br><font size=2>This name will be printed as HTML source, so you can include any HTML";
echo "<br>code that you want. This will only be displayed in your teams' page.";
echo "<br>If you don't know HTML, just leave this box blank.";
echo "</td></tr><tr>";
echo "<td>URL of team web page, if any:<br><font size=2>(without &quot;http://&quot;)</td>";
echo "<td><input name=url size=60 value=$team_url></td></tr><tr>";
echo "<td valign=top>Description of team:</td>";
echo "<td><textarea name=description value=$team_description cols=60 rows=10>$team_description</textarea>";
echo "</td></tr><tr>";
echo "<td valign=top>Type of team:</td><td>";
printf("<input type=radio name=type value=4%s>Club<br>", ($team->type==4)?" checked":"");
printf("<input type=radio name=type value=1%s>Small Company (< 50 employees)<br>", ($team->type==1)?" checked":"");
printf("<input type=radio name=type value=2%s>Medium Company (50-1000 employees)<br>", ($team->type==2)?" checked":"");
printf("<input type=radio name=type value=3%s>Large Company (> 1000 employees)<br>", ($team->type==3)?" checked":"");
printf("<input type=radio name=type value=5%s>Primary School<br>", ($team->type==5)?" checked":"");
printf("<input type=radio name=type value=6%s>Secondary School<br>", ($team->type==6)?" checked":"");
printf("<input type=radio name=type value=8%s>Junior College<br>", ($team->type==8)?" checked":"");
printf("<input type=radio name=type value=7%s>University or Department<br>", ($team->type==7)?" checked":"");
printf("<input type=radio name=type value=9%s>Government Agency</td>", ($team->type==9)?" checked":"");
echo "</tr></table>";
echo "<input type=submit value=\"Edit Team\">";
echo "</form>";
}

function print_team_remove_inactive_form($id) {

$query = sprintf(
    "select * from team where id = %d",
    $id
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
$nusers = $team->nusers;
page_head("Remove Members from $team_name");
echo "<h2>Remove members from $team_name</h2>";
echo "<p><b>Please note:</b>";
echo "<ul>";
echo "<li>Only the founder can remove members from a team";
echo "<li>By removing a member, you will also remove their credit and CPU time ";
echo "contributions to the team.";
echo "<li>Team members may only be removed if they have been inactive for ";
echo "more than two weeks.";
echo "</ul>";
echo "</p>";
echo "<hr>";
echo "<form method=post action=team_remove_inactive_action.php>";
echo "<input type=hidden name=id value=$team_id>";
echo "<br></td></tr></table>";
echo "<table border=1><tr><th>Remove?</th>";
echo "<th>Name</th>";
echo "<th>Total<br>Credit</th>";
echo "<th>Inactive Since</th></tr>";

$query = sprintf(
    "select * from user where teamid = %d",
    $team_id
);
$result = mysql_query($query);

$ninactive_users = 0;
for ($i = 0; $i < $nusers; $i++) {
    $user = mysql_fetch_object($result);
    if ((time() - $user->inactive_since) > 1209600) {
        // 1209600 is the number of seconds in two weeks
        echo "<tr><td align=center><input type=checkbox name=remove_$ninactive_users value=$user->id>";
        echo "<td>$user->name</td>";
        echo "<td>$user->total_credit</td>";
        echo "<td>$user->inactive_since</td>";
        $ninactive_users++;
    }
}
echo "<input type=hidden name=ninactive_users value=$ninactive_users>";
if ($result) {
   mysql_free_result($result);
}
echo "</table>";
echo "<input type=submit value=\"Remove Users\">";
echo "</form>";
}

function print_team_disband_form($id) {

$query = sprintf(
    "select * from team where id = %d",
    $id
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
    mysql_free_result($result);
}
$team_name = $team->name;
$team_id = $team->id;
page_head("Disband $team_name");
echo "<h2>Disband $team_name</h2>";
echo "<p><b>Please note:</b>";
echo "<ul>";
echo "<li>Only the found can disband a team.";
echo "<li>Only teams with no members can be disbanded. The founder of a team ";
echo "may email the members of a team to inform them of the disbanding of the team.";
echo "<li>After a team is disbanded, all memeber will no longer belong to a team, ";
echo "though they may join another team if desired.";
echo "<li>Disbanding a team is permanent, make sure you know what you're doing!";
echo "</ul>";
echo "</p>";
echo "Press &quot;Disband Team&quot; when you're ready.";
echo "<hr>";
echo "<form method=post action=team_disband_action.php>";
echo "<input type=hidden name=id value=$team_id>";
echo "<input type=submit value=\"Disband Team\">";
echo "</form>";
}

?>
