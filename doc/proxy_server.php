<?php
require_once("docutil.php");
page_head("Proxy servers");
echo "
The original SETI@home benefited from the development
of 'Proxy servers' such as SETIQueue.
These proxy servers buffer work units and results
between participant computers and the main SETI@home server.
They provide a smooth supply of work even when the main server is down,
and they make it possible to run SETI@home on computers
not connected directly to the Internet.
<p>
Things are trickier in BOINC.
Unlike SETI@home, with its 'one size fits all' work units,
BOINC allows work units that have extreme requirements
(memory, disk, CPU) and makes sure they're sent only
to hosts that can handle them.
In BOINC, a client communicates directly with the server,
telling the server about its hardware (memory size, CPU speed etc.)
and the server chooses work for it accordingly.
Furthermore, BOINC has separate scheduling and data servers
(in SETI@home, a single server played both roles).
<p>
So a BOINC proxy would have to replicate much
of the functionality of the BOINC core client
(so that it can download and upload files)
and the BOINC scheduling server
(since it would have to implement the work-distribution policy).
This is possible but it would be a lot of work.
<p>
BOINC has mechanisms - such as work buffering
and the ability to participate in multiple projects -
that reduce the importance of proxy servers.
";
page_tail();
?>
