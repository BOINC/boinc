<?php
require_once("docutil.php");
page_head("Which applications are suitable for BOINC?");
echo "
<p>
BOINC is designed to support applications
that have large computation requirements,
storage requirements, or both.
If a project can attract a million participants,
it could gain access to dozens of TeraFLOPs of CPU power
and perhaps a Petabyte of storage.

<p>
However, because the resources of a BOINC project are
unreliable and sporadically-connected Internet PCs,
an application must have several properties to effectively use BOINC:

<dl>
<dt>
<b>Public appeal</b>
<dd>
An application must be viewed as interesting
and worthwhile by the public in order to gain large numbers of participants.
A project must have the resources and commitment to maintain this interest,
typically by creating a compelling web site
and by generating interesting graphics in the application.
<dt>
<b>Independent parallelism</b>
<dd>
The application must be divisible into parallel parts with
few or no data dependencies.
<dt>
<b>Low data/compute ratio</b>
<dd>
Input and output data are sent through commercial Internet connections,
which may be expensive and/or slow.
As a rule of thumb,
if your application produces or consumes more than
a gigabyte of data per day of CPU time,
then it may be cheaper to use in-house cluster computing.
<dt>
<b>Fault tolerance</b>
<dd>
A result returned from a public-resource computer
cannot be assumed to be correct.
Redundant computing can be used to reduce the error probability,
but not all the way to zero.
If your application relies on 100% correctness,
you shouldn't use BOINC.

</dl>
";
page_tail();
?>
