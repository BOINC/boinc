<?php
require_once("docutil.php");
page_head("CPU scheduling in Version 4+");

echo "
<h3>Time-slicing</h3>
<p>
Starting with version 4.00,
the BOINC core client does <b>time-slicing</b>.
This means that the core client may switch back and forth
between results of different projects.
This is done in a way that allocates CPU time
according to the 'resource shares' you have assigned to each project.
<p>
For example, suppose you participate in SETI@home
with resource share 100
and Predictor@home with resource share 200.
A single-processor machine might be scheduled as follows:
<pre>
1:00 - 2:00: SETI@home
2:00 - 3:00: Predictor@home
3:00 - 4:00: Predictor@home
4:00 - 5:00: SETI@home
5:00 - 6:00: Predictor@home
6:00 - 7:00: Predictor@home
...
</pre>
A two-processor machine might be scheduled as follows:
<pre>
             CPU 0             CPU 1
1:00 - 2:00: Predictor@home    SETI@home
2:00 - 3:00: Predictor@home    SETI@home
3:00 - 4:00: Predictor@home    Predictor@home
4:00 - 5:00: Predictor@home    SETI@home
5:00 - 6:00: Predictor@home    SETI@home
6:00 - 7:00: Predictor@home    Predictor@home
</pre>
In every 3 hour period,
your computer spends 4 hours on Predictor@home
and 2 hours on SETI@home, which is the desired ratio.

<p>
This feature is necessary to handle projects like
<a href=http://climateprediction.net>Climate<i>prediction</i>.net</a>,
whose work units take a long time (1 or 2 months)
to complete on a typical computer.
Without time-slicing, your computer would have to finish an
entire work unit before it could start working on a different project.

<h3>Preemption</h3>
<p>
When BOINC switches from one application to another,
the first application is said to be <b>preempted</b>.
BOINC can do preemption in two different ways;
you can select this as part of your
<a href=prefs.php>General Preferences</a>.
<ul>
<li>
<b>Don't leave the suspended applications in memory</b> (default).
Applications are preempted by killing them;
they are later restarted, and resume from their last checkpoint.
This saves virtual memory (swap space) but can waste CPU time,
especially if applications checkpoint infrequently.
<li>
<b>Leave suspended applications in memory</b>.
Applications are preempted by suspending them;
they remain in virtual memory while preempted
(they don't necessarily occupy physical memory).

";
   page_tail();
?>

