<?php
require_once("docutil.php");

page_head("Application development");
echo "
<h2>Cross-platform functions</h2>
<p>
Most POSIX calls are supported on Unix and Windows.
For areas that are different (e.g. scanning directories)
BOINC supplies some generic functions with implementations for all platforms.
Similar code may be available from other open-source projects.

<h2>Stack traces</h2>
<p>
You can use Stackwalker to generate symbolic stack traces
if your application crashes.
These traces will be returned in the
'stderr_out' field of results.

<h2>Windows-specific issues</h2>
<p>
The set of 'standard' DLL differs somewhat among
9X/NT/2000/XP.
To avoid crashing because a DLL is missing,
call ::LoadLibrary() and then get function pointers.

<h2>Unix-specific issues</h2>
<p>
static/dynamic linking

<h2>Cross-language issues</h2>
<p>
How to access BOINC API from C, FORTRAN?
";
page_tail();
?>
