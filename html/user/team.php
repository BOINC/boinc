<?php

include_once("db.inc");
include_once("util.inc");
include_once("team.inc");

init_session();
db_init();

    page_head("Teams");

echo "<p>".PROJECT." participants may form <b>teams</b>.
    <p>
    You may belong to only one team.
    You can join or quit a team at any time.
    <p>
    Each team has a <b>founder</b>, who may
    <ul>
    <li> access team members' email addresses.
    <li> edit the team's name and description,
    <li> remove members from the team, and
    <li> disband a team if it has no members.
    </ul>
    <p>
    <form method=post action=team_lookup.php>
    Search for a team whose name contains:
    <input name=team_name>
    <input type=submit name=search value=Search>
    </form>
    You may join a team on its team page.
    <p>
    <a href=top_teams.php>Show top teams</a>
    <p>
    <a href=team_create_form.php>Create a team</a>
    <p>
";
    page_tail();

?>
