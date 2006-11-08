<?php
require_once("docutil.php");

page_head("Application development tips");
echo "
<h2>Cross-platform functions</h2>
<p>
Most POSIX calls are supported on Unix and Windows.
For areas that are different (e.g. scanning directories)
BOINC supplies some generic functions with implementations for all platforms.
Similar code may be available from other open-source projects.

<p>
LIST THEM

<h2>Windows-specific issues</h2>
<ul>
<li>
The set of 'standard' DLL differs somewhat among 9X/NT/2000/XP.
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

<h2>Cross-language issues</h2>
<p>
The BOINC API is implemented in C++.
Information about using it from C and FORTRAN is
<a href=fortran.php>here</a>.
";
page_tail();
?>
