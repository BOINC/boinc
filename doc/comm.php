<?php
require_once("docutil.php");
page_head("BOINC network communication overview");
echo"

<p>
The BOINC core client communicates with several servers
in the course of getting work and returning results.
All communication uses HTTP on port 80,
so client can function through firewalls and proxies.

<img src=comm.png>

<ul>
<li>
The client downloads the page from project's
<a href=server_components.php>master URL</a>.
From XML tags embedded in this page,
it obtains a list of domain names of <b>schedulers</b>.
<li>
The client exchanges
<a href=protocol.php>request and reply messages</a>
with a scheduling server.
The reply message contains, among other things,
descriptions of work to be performed,
and lists of URLs of the input and output files of that work.
<li>
The client downloads files (application programs and data files)
from one or more <b>download data servers</b>.
This uses standard HTTP GET requests, perhaps with <code>Range</code> commands
to resume incomplete transfers.
<li>
After the computation is complete,
the client uploads the result files.
This uses a <a href=upload.php>BOINC-specific protocol</a>
that protects against DOS attacks on data servers.
<li>
The client then contacts a scheduling server again,
reporting the completed work and requesting more work.
</ul>
";
page_tail();
?>
