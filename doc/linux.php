<?php

require_once("docutil.php");

page_head("Running BOINC on Linux");

echo "
The BOINC client software should work on:

<ul>
<li> Any Linux version 2.0 or greater with libc version 2.2 or greater.
(If BOINC produces the error message
<pre>
boinc_client: /lib/libc.so.6: version `GLIBC_2.2' not found (required by ./boinc_client)
</pre>
then you need to install a newer version of libc.)

<li> Any Intel x86-compatible processor.
</ul>
";

page_tail();
?>
