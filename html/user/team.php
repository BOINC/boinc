<?php

include_once("db.inc");
include_once("util.inc");
include_once("team.inc");

$authenticator = init_session();
db_init();

    page_head("Teams");
?>

<p>
Participants in this project may form "teams" -
for example, students in a school, employees of a company, etc.
<ul>
<li> You may join a team at any time.
<li> You may quit your current team at any time.
<li> Only the team founder may edit a team's fields (name, description, etc.).
<li> The founder has the right to remove members from a team.
<li> The founder may disband a team if it has no members.
A team may not be disbanded if it still has members.
<li> The founder has access to all team members' email addresses.
</ul>
<p>
<a href=top_teams.php>Show top teams</a>
<p>
<a href=team_create_form.php>Create a new team</a>
<p>
To search for a team type in the team name below and hit the Search button.
You may join teams through these team pages.
<p>
<form method=post action=team_lookup.php>
Search for a team whose name contains:
<input name=team_name>
<input type=submit name=search value=Search>
</form>

<?php
    page_tail();

?>
