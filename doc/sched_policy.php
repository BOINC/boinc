<?php
require_once("docutil.php");
page_head("Work distribution policy");
echo "
<p>
The scheduling server will attempt to send enough work to exceed a
hosts high water mark.  If the amount of work the scheduling server is
sending exceeds a certain level (default is four weeks), the
scheduling server will not attach more work to a scheduler reply.
This does not prevent the scheduling server from sending a lengthy
workunit, but rather from sending multiple lengthy workunits.
<p>
If a work unit uses more disk resources than a host has available, the
scheduling server will not attach that work unit.
<p>
The scheduling server
estimates the amount of time a work unit will take to complete with the
formula <b>(number of flops)/(flops per second)+(number of iops)/(iops
per second)</b>.
The number of floating point and integer operations are provided by
the project when creating the work unit, and the host calculation
speeds are included in a scheduler request.
<p>
If no work is available,
or if the host cannot accept it for whatever reason
(too slow, not enough space, etc),
the scheduling server sends the message
<b>no work available</b>,
and requests that the client wait before sending another request.
";
page_tail();
?>
