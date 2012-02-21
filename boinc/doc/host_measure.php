<?php
require_once("docutil.php");
page_head("Host measurements");
echo "
<p>
The core client measures the following aspects of each host: 
<ul>
<li> <b>CPU performance</b>: integer ops/sec, double-precision
floating-point ops/sec, and memory bandwidth are measured separately.
They are measured by a process executing at the same priority as BOINC
applications, so the results will be affected by other processes.
These measurements are taken when the client starts for the first time,
and once every month thereafter.
<li> <b>Number of CPUs</b>:  By default, the number of simultaneous
slot directories will be set to this number unless otherwise indicated
by user preferences.
<li> <b>Vendor and model of CPU</b>
<li> <b>Disk space</b>: free space and total space on the drive which
BOINC is installed on.
These numbers will be used to ensure BOINC does
not use more space than set in the user preferences.
<li> <b>Memory</b>: total RAM, CPU cache, and swap space.
These numbers can be used by the scheduling server to decide whether or not to
assign work to a client.
This also provides a means for assignment of
differing work based on host abilities.
<li> <b>Timezone</b>: 
<li> <b>Last IP address</b> and count of consecutive same addresses.
<li> <b>Number of RPCs, and time of last RPC</b>.
<li> <b>Fractions of time</b> that core client runs on host, host is
connected, and user is active.
These are computed as exponentially-weighted averages; see the class TIME_STATS.
<li> <b>Operating system name and version</b>.
<li> <b>Average up- and downstream network bandwidth</b>.
These are computed as exponentially-weighted averages; see the class NET_STATS.
</ul>

These quantities are reported in every scheduling RPC, and their latest
values are stored in the BOINC database.

";
page_tail();
?>
