<?
require_once("docutil.php");
page_head("Computation credit");
echo "

<p>
Each project gives you <b>credit</b> for the computations your
computers performs for the project.

<p>
BOINC's credit system is based on a 'reference computer' that can do
<ul>
<li>1 billion floating-point multiplies per second
<li>1 billion integer multiplies per second
<li>4 billion bytes per second of traffic to and from main memory
(sequential, half reads and half writes)
</ul>
BOINC's unit of credit, the <b>Cobblestone</b> <sup>1</sup>,
is 1/300 day of CPU time on the reference computer.

<p>
Some BOINC projects grant credit only after
results have been <a href=intro_user.php#credit>validated</a>.

<p>
Each project maintains two types of credit:
<ul>
<li> <b>Total credit</b>:
The total number of Cobblestones performed.
<li> <b>Recent average credit</b>:
The average number of Cobblestones per day performed recently.
This average decreases by a factor of two every week,
according to algorithms given below.
</ul>

<p>
Both types of credit (total and recent average)
are maintained for each user and host.

<h3>Leader boards</h3>

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

<h3>How 'Recent Average Credit' is computed</h3>
Each time new credit granted,
the following function is used to update the
recent average credit of the host, user and team:
<pre>",htmlspecialchars("

// Update an estimate of \"units per day\" of something (credit or CPU time).
// The estimate is exponentially averaged with a given half-life
// (i.e. if no new work is done, the average will decline by 50% in this time).
// This function can be called either with new work,
// or with zero work to decay an existing average.
//
void update_average(
    double work_start_time,       // when new work was started
                                    // (or zero if no new work)
    double work,                    // amount of new work
    double half_life,
    double& avg,                    // average work per day (in and out)
    double& avg_time                // when average was last computed
) {
    double now = dtime();

    if (avg_time) {
        double diff = now - avg_time;
        double diff_days = diff/SECONDS_PER_DAY;
        double weight = exp(-diff*M_LN2/half_life);
        avg *= weight;
        avg += (1-weight)*(work/diff_days);
    } else {
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}

"),"
</pre>
<hr noshade size=1>
<sup>1</sup> Named after Jeff Cobb of SETI@home
";
page_tail();
?>
