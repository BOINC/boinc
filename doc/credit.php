<?
require_once("docutil.php");
page_head("Computation credit");
echo "

<p>
Each project gives you <b>credit</b> for the computations your
computers performs for the project.
These credits are used to generate web-site 'leaderboards' showing
individuals, teams, and categories (countries, CPU types, etc.)
ranked by credit.

<p>
BOINC's credit system is based on a 'reference computer' that does
<ul>
<li>1 billion floating-point multiplies per second
<li>1 billion integer multiplies per second
<li>4 billion bytes per second of traffic to and from main memory
(sequential, half reads and half writes)
</ul>
BOINC's unit of credit, the <b>Cobblestone</b> <sup>1</sup>,
is one day of CPU time on the reference computer.

<p>
Each project maintains two types of credit:
<ul>
<li> <b>Total credit</b>:
The total number of Cobblestones performed.
<li> <b>Recent average credit</b>:
The average number of Cobblestones per day performed recently.
This average decreases by a factor of two every week.
</ul>

<p>
Both types of credit (total and recent average)
are maintained for each user and host.

<h3>Leader boards</h3>
The PHP pages supplied by BOINC include basic leaderboards:
top users and hosts, by total and average.
BOINC lets projects export the credit-related
parts of their database as XML files.
These XML files can be used to generate
other breakdowns of users, hosts and teams,
or to generate leaderboards based on the sum of
credit from different projects.


<h3>Possible future improvements</h3>
<ul>
<li>
Ideally, credit should reflect network transfer and disk storage as well
as computation.
But it's hard to verify these activities,
so for now they aren't included.
<li>
Eventually projects will develop applications that use
graphics coprocessors or other non-CPU hardware.
Credit should reflect the usage of such hardware.
To accomplish this, we will need to let
projects supply their own benchmarking functions.
This will also handle the situation where a project's
application does e.g. all integer arithmetic.
</ul>
<hr noshade size=1>
<sup>1</sup> Named after Jeff Cobb of SETI@home
";
page_tail();
?>
