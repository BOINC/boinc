<?php
require_once("docutil.php");
page_head("Proxy servers");
echo "
SETI@home Classic benefited from 'proxy servers' such as SETIQueue,
that store work units and results,
and transfer them
between participant computers and the main SETI@home server.
Proxies provide a smooth supply of work even when the main server is down,
and they make it possible to run SETI@home Classic on computers
not connected directly to the Internet.
<p>
These programs won't work with BOINC (see below),
but some of their benefits can be achieved in other ways:
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

<h3>How a BOINC proxy system might work</h3>
<p>
Here's a sketch of a proxy system based on a modified core client.
We assume that there's a 'proxy' host that
does only communication and storage,
and a number of 'worker' hosts that do computation.
The core client must be modified to accept -proxy and -worker options:
<ul>

<li> With the -proxy option,
the client does network communication (scheduler RPC, file upload and download)
and no computation, CPU benchmarking,
or measurement of other hardware info like memory and disk size.
(It does, however, measure and store network speed.)
It exits when network communication is finished.

<li> With the -worker option,
the client does the complement:
computation and CPU benchmarking but no network communication, etc.
It exits when computation is finished
(or perhaps when a CPU becomes idle,
or when a project is starved).
</ul>

The proxy host would maintain a set of separate BOINC directories,
one for each worker host.
The high-level logic is (for each worker host):
<ul>
<li> Run the core client with -worker on the worker host.
<li>
When it exits, synchronize its directory with
the corresponding directory on the proxy host.
<li>
Run the core client with -proxy on the proxy host.
<li>
When it exits, synchronize its directory with
the corresponding directory on the worker host.
<li>
Repeat.
</ul>
Note: none of the following is implemented.
If you are a programmer and would like to help, please let us know.
Also note: as described above, the system is not asynchronous
(computation and communication don't overlap)
and the proxy doesn't act as a buffer.
It could be modified to have these properties.

";
page_tail();
?>
