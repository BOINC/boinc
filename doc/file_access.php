<?php
require_once("docutil.php");
page_head("Remote file access");
echo "
<p>
Output files can be designated as sticky but not upload-when-present.
Sometimes this is done because the file will be
used as an input for a later workunit; it may also be done to spread
large output data across participant disks, in cases where the data may
be too large to store on the project back-end. 
<p>
To access such a file, the back end inserts a <b>file access
request</b> in the database.
This specifies a file and a data-server URL.
When a host storing the file contacts the scheduling server (which
may not happen for a while) the scheduling server forwards the file
access request.
The host confirms the file transfer when it is
completed; the back-end must poll for this event. 
";
page_tail();
?>
