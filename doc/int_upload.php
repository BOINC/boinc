<?php
require_once("docutil.php");
page_head("Intermediate upload");
echo "
Long-running applications can upload their output files
before the result as a whole is finished.
To initiate the upload of an output file, call
<pre>
   extern int boinc_upload_file(std::string& name);
</pre>
where 'name' is the logical name of the file.
The application cannot modify the file after making this call.
<p>
To check on the status of a file being uploaded, call
<pre>
   extern int boinc_upload_status(std::string& name);
</pre>
This will return zero if the upload of the file is finished
successfully.

";

page_tail();
?>
