<?php
require_once("docutil.php");
page_head("Teams");
echo "

<p>
Participants in a project can form <b>teams</b>.
Each participant can belong to at most one team.
<p>
A team is local to a project.
There is currently no provision for teams that span projects.
<p>
Total and average credit is accounted for teams,
and reflects all work done by participants
while they belong to the team
(even if they later quit the team).

<p>
A team has the following attributes:
<ul>
<li> A textual name.
<li> An HTML name (can include graphics).
<li> A textual description.
<li> An optional URL (e.g., of the team's web site)
<li> A type (business, school, club, etc.).
<li> The <b>founder</b>: initially, this is the user who
created the team, but it may change (see below).
A team's founder may quit the team,
but will remain as its founder.
<li> A list of members.
</ul>

<p>
A project's web site lets you:
<ul>
<li> See lists of the teams with the most credit.
<li> Search for teams in various ways.
<li> Join a team
<li> Quit your current team
</ul>

<p>
The founder of a team has some additional capabilities:
<ul>
<li> Edit the team's attributes.
<li> View the email addresses of all team members.
<li> Remove members from the team.
<li> Change the founder (to any current member).
<li> Disband the team.
</ul>
";
page_tail();
?>
