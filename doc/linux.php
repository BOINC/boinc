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

<h3>64 bit client</h3>

On a 64-bit Linux system you should use the
64-bit version of the BOINC client
(its benchmarks will be more accurate than those of the 32-bit version).

<p>
NOTE: the 64-bit BOINC client will download 32-bit applications
from projects that don't have 64-bit apps.
Note that 32-bit binaries don't just work on every 64-bit Linux. If
for example you install a fresh Ubuntu 6.10 or 7.04, 32-bit binaries
won't work. They are not even recognized as valid executables. You
first have to install the ia32 package and dependent packages.
Further, for programs that link with the graphic library, you will
manually have to copy a 32-bit libglut library to the usr/lib32
directory.
If after this you still get client errors,
find your exe in the projects directory and run ldd to see what
libraries are missing.

";

page_tail();
?>
