<?php
require_once("docutil.php");
page_head("Proxy servers");
echo "
SETI@home Classic benefited from 'proxy servers' such as SETIQueue.
These proxy servers buffer work units and results
between participant computers and the main SETI@home server.
They provide a smooth supply of work even when the main server is down,
and they make it possible to run SETI@home Classic on computers
not connected directly to the Internet.
<p>
These programs won't work with BOINC (see below),
but some of their functions can be performed by other means:
<ul>
<li>
The buffering of multiple work units is provided by the BOINC client itself -
you can specify how much work your computer should get
each time it contacts the server.
<li>
Hosts that are not directly connected to the Internet,
but share a LAN with one that is,
can participate in BOINC using an HTTP 1.0 proxy
such as <a href=http://www.squid-cache.org/>Squid</a> for Unix or
<a href=http://download.com.com/3000-2381-10226247.html?tag=lst-0-21>FreeProxy</a> for Windows.

</ul>

<h3>Why won't SETIQueue work with BOINC?</h3>
<p>
Unlike SETI@home Classic, with its 'one size fits all' work units,
BOINC allows work units that have extreme requirements
(memory, disk, CPU) and makes sure they're sent only
to hosts that can handle them.
In BOINC, a client communicates directly with the server,
telling the server about its hardware (memory size, CPU speed etc.)
and the server chooses work for it accordingly.
Furthermore, BOINC has separate scheduling and data servers
(in SETI@home Classic, a single server played both roles).
<p>
So a BOINC proxy would have to replicate much
of the functionality of the BOINC core client
(so that it can download and upload files)
and the BOINC scheduling server
(since it would have to implement the work-distribution policy).
This is possible but it would be a lot of work.
";
page_tail();
?>
