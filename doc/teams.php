<?
require_once("docutil.php");
page_head("Teams");
echo "

<p>
Participants in a project can form <b>teams</b>.
Each participant can belong to at most one team.
The participant who created the team is called its <b>founder</b>.
<p>
A team is local to a project.
There is currently no provision for teams that span projects.
<p>
Total and average credit is accounted for teams;
a team's credit is the sum of the credits of its members.

<p>
A team has the following attributes:
<ul>
<li> A textual name.
<li> An HTML name (can include graphics).
<li> A textual description.
<li> An optional URL (e.g., of the team's web site)
<li> A type (business, school, club, etc.).
<li> The founder.
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
<li> Disband the team.
</ul>
";
page_tail();
?>
