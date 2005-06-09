<?php
require_once("docutil.php");
page_head("Testing the BOINC client");
echo "

<p>The following tests are meant for the single-user installation scenario:
<ol>
<li>Attach to a project.
<li>Schedule an RPC to the project server requesting results to process.
<li>Download the project executable and support files.
<li>Download a result and support files.
<li>Begin execution of the project executable using the retrieved result and support files.
<li>Complete a result and upload the resulting file(s) of the result execution.
<li>Accept the update GUI RPC to tell the client to report the completed result to the scheduler.
<li>Server Scheduler Exponential Back-off on connection failure.
<li>File download Exponential Back-off on transient connection failure.
<li>Server Scheduler Back-off with a requested value.
<li>Server Scheduler error message reported to client.
<li>File download resumes after back-off completes.
<li>File upload resumes after back-off completes.
</ol></p>

";
page_tail();
?>
