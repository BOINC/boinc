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
<ul>
<li>
The set of 'standard' DLL differs somewhat among
9X/NT/2000/XP.
To avoid crashing because a DLL is missing,
call ::LoadLibrary() and then get function pointers.
<li>
Visual Studio: set 'Create/Use Precompiled Header' to
'Automatically Generate' (/YX)
in C/C++ Precompiled Header project properties.
<li>
Visual Studio: change 'Compile As' to
'Compile as C++ Code (/TP)'
in C/C++ 'Compile As' project properties.
</ul>

<h2>Unix-specific issues</h2>
<p>
Most important: use gcc-3.0, at least for linking.
This should limit the executable to GLIBC-2.2 symbols.
Einstein@Home builds Apps on a Debian Sarge (3.1) system,
using the gcc-3.0 from Woody.
Take special care of which libs to link statically and which dynamically
if you are using e.g. OpenGL.

<p>
For a portable build of the BOINC Core Client I found no better way
than to make a clean Debian Woody installation (in a VMWare machine) from DVD,
refusing all online updates (of the GLIBC).
However this was quite some time ago, before BOINC switched
to ssl, curl and zlib, I don't know if and how this still works. 

<h2>Cross-language issues</h2>
<p>
The BOINC API is implemented in C++.
Information about using it from C and FORTRAN is
<a href=fortran.php>here</a>.
";
page_tail();
?>
