<?php

require_once("docutil.php");

page_head("Running BOINC on Linux");

echo "
The BOINC client software should work on:

<ul>
<li> Any Linux version 2.0 or greater with libc version 2.2 or greater.
<li> Any Intel x86-compatible processor.
</ul>

<p>
Specifically, it should work on Redhat 7.3 or later.
<hr>
If BOINC produces as error message of the form
<pre>
boinc_client: /lib/libc.so.6: version `GLIBC_2.2' not found (required by ./boinc_client)
</pre>
then you need to install a newer version of libc.
<hr>
If BOINC produces as error message of the form
<pre>
boinc_client: /lib/libc.so.6: version `GLIBC_2.3' not found (required by ./boinc_client)
</pre>
then the BOINC client was built incorrectly;
please alert us.

";

page_tail();
?>
