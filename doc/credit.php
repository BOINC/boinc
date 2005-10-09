<?php
require_once("docutil.php");
page_head("Computation credit");
echo "

<p>
A BOINC project gives you <b>credit</b> for the computations your
computers perform for it.
BOINC's unit of credit, the <b>Cobblestone</b> <sup>1</sup>,
is 1/100 day of CPU time on a reference computer that does
<ul>
<li> 1,000 double-precision MIPS based on the Whetstone benchmark.
<li> 1,000 VAX MIPS based on the Dhrystone benchmark.
</ul>
These benchmarks are <a href=benchmark.php>imperfect predictors</a>
of application performance, but they're good enough.
<p>
Eventually, credit may reflect network transfer and disk storage as well
as computation.

<h2>How credit is determined</h2>

When your computer completes a result,
BOINC determines an amount of <b>claimed credit</b>
in one of two ways:
<ul>
<li>
In general, the claimed credit is the result's CPU time multiplied
by the CPU benchmarks as measured by the BOINC software.
<b>NOTE:</b> the BOINC software is not optimized for specific processors.
Its benchmark numbers may be lower than
those produced by other programs.
<li>
Some applications determine claimed credit themselves,
and report it to BOINC.
This would be the case, for example, with
applications that use graphics coprocessors or other non-CPU hardware.
</ul>

<p>
Claimed credit is reported to a project
when your computer communicates with its server.
The <b>granted credit</b> that you receive may
be different from the claimed credit,
and there may be a delay of a few hours or days
before it is granted.
This is because some BOINC projects grant credit only after
results have been <a href=intro_user.php#credit>validated</a>.



<h2>Recent Average Credit</h2>
<p>
Projects maintain two counts of granted credit:
<ul>
<li> <b>Total credit</b>:
The total number of Cobblestones performed and granted.
<li> <b>Recent average credit</b>:
The average number of Cobblestones per day granted recently.
This average decreases by a factor of two every week,
according to the algorithm given below.
</ul>

<p>
Both quantities (total and recent average)
are maintained for each user, host and team.

<p>
Each time new credit is granted,
the following function is used to update the
recent average credit of the host, user and team:
<pre>",htmlspecialchars("


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
        // If an average R already exists, imagine that the new work was done
        // entirely between avg_time and now.
        // That gives a rate R'.
        // Replace R with a weighted average of R and R',
        // weighted so that we get the right half-life if R' == 0.
        //
        // But this blows up if avg_time == now; you get 0*(1/0)
        // So consider the limit as diff->0,
        // using the first-order Taylor expansion of
        // exp(x)=1+x+O(x^2).
        // So to the lowest order in diff:
        // weight = 1 - diff ln(2) / half_life
        // so one has
        // avg += (1-weight)*(work/diff_days)
        // avg += [diff*ln(2)/half_life] * (work*SECONDS_PER_DAY/diff)
        // notice that diff cancels out, leaving
        // avg += [ln(2)/half_life] * work*SECONDS_PER_DAY

        double diff, diff_days, weight;

        diff = now - avg_time;
        if (diff<0) diff=0;

        diff_days = diff/SECONDS_PER_DAY;
        weight = exp(-diff*M_LN2/half_life);

        avg *= weight;

        if ((1.0-weight) > 1.e-6) {
            avg += (1-weight)*(work/diff_days);
        } else {
            avg += M_LN2*work*SECONDS_PER_DAY/half_life;
        }
    } else if (work) {
        // If first time, average is just work/duration
        //
        double dd = (now - work_start_time)/SECONDS_PER_DAY;
        avg = work/dd;
    }
    avg_time = now;
}
"),"
</pre>

<h3>Computing the current value of Recent Average Credit</h3>
<p>
BOINC updates 'recent average credit' (RAC) only when new credit is granted.
Interfaces that export RAC also export that time at which it was last updated.
To obtain the current value of RAC,
you must 'decay' it based on the time that has elapsed
since it was updated: <pre>", htmlspecialchars('
function decay_average($avg, $avg_time, $now = 0) {
   $M_LN2 = 0.693147180559945309417;
   $credit_half_life = 86400 * 7;
   if ($now == 0) {
       $now = time();
   }
   $diff = $now - $avg_time;
   $weight = exp(-$diff * $M_LN2/$credit_half_life);
   $avg *= $weight;
   return $avg;
}
'), "</pre>
<p>
If you don't apply this decay,
inactive entities will have incorrectly high RAC.

<p>
PHP code for the decay function can be found in
html/inc/credit.inc and html/inc/host.inc.

<hr noshade size=1>
<sup>1</sup> Named after Jeff Cobb of SETI@home
";
page_tail();
?>
