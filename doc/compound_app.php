<?php
require_once("docutil.php");

page_head("Compound applications");

echo "
<h3>Compound applications</h3>

A <b>compound application</b> consists of a <b>main program</b>
and one or more <b>subsidiary programs</b>.
The main program executes the subsidiary programs in sequence.
It maintains a state file that records
which subsidiary programs have already completed.
It assigns to each subsidiary application
a subrange of the overall 'fraction done' range of 0..1.
For example, if there are two subsidiary applications
with equal runtime,
the first would have range 0 to 0.5 and the second
would have range 0.5 to 1.


<p>
The main program must not call boinc_init() or boinc_finish().
Its logic is as follows:
<pre>
read state file
for each remaining subsidiary application:
    boinc_parse_init_data_file()
    aip.fraction_done_start = x
    aip.fraction_done_end = y
    boinc_write_init_data_file()
    run the app
    write state file
exit(0)
</pre>
where x and y are the appropriate fraction done range limits.

<p>
Each subsidiary program should use the normal BOINC API,
including calls to boinc_fraction_done()
with values ranging from 0 to 1.

";
page_tail();
?>
