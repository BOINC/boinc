<?
require_once("docutil.php");
page_head("Persistent file transfer");
echo "
<p>
A file upload or download may experience various types of
<b>transient failures</b>:
<ul>
<li> One or more data servers have failed.
<li> A network connection fails.
<li> The host PC is turned off or the core client quits.
</ul>
BOINC uses a mechanism called <b>persistent file transfer</b>
for efficiently recovering from these conditions,
and deciding when a <b>permanent failure</b> has occurred.
<p>
The FILE_XFER class encapsulates a single transfer session
to a particular data server.
If the file has previously partially transferred,
FILE_XFER resumes at the appropriate point.
<p>
The PERS_FILE_XFER class encapsulates a persistent file transfer,
which may involve a sequence of FILE_XFERs,
possibly to different data servers.
<p>
When a file is involved in a persistent file transfer,
the state is saved in the client state file
in the following XML element
(included in the &lt;file_info&gt; element):
<pre>
&lt;persistent_file_xfer>
    &lt;num_retries>2&lt;/num_retries>
    &lt;first_request_time>1030665600&lt;/first_request_time>
    &lt;next_request_time>1030665725&lt;/next_request_time>
&lt;/persistent_file_xfer>
</pre>
<ul>
<li> The <b>num_retries</b> element is the number of
transfer sessions so far.
<li> The <b>first_request_time</b> element is the time
the first transfer session started.
<li> The <b>next_request_time</b> element is the earliest time
to start a new transfer session.
</ul>
<p>
When there is a transient failure, the core client
increments num_retries and calculates a new next_request_time
based on randomized exponential backoff, given by

<pre>
next_request_time = current_time+max(MIN_DELAY,min(MAX_DELAY,exp(rand(0,1)*num_retries)))
</pre>
Where MIN_DELAY is 1 minute and MAX_DELAY is 4 hours.
<p>
The client classifies the transfer as a permanent failure if the
current time becomes much later than this (default is two weeks).

??? later than what?

In this event, the file will be deleted and the failure reported
to the scheduling server.

??? in what form?
";
page_tail();
?>
