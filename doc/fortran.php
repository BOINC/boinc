<?php
require_once("docutil.php");
page_head("FORTRAN applications");?>

<h2>Windows: cygwin</h2>
<p>
Include the file 'boinc_api_fortran.C' in the api/Makefile.am,
but comment out the 'zip' calls, to avoid the linking with 'libboinc_zip.a' 
<p>
To link is necessary to add the 'winmm.dll' (-lwinmm).
<h2>Windows: Visual Developer Studio</h2>

<p><font color=red>2004-06-16 note: this page is outdated; will update
    (functions are now declared <code>extern"C"</code> so
     no C++ mangling is done; there is a boinc_api_fortran.C wrapper) -- quarl@ssl
    </font></p>

<p>
Note: a working example similar to the following
(based on outdated BOINC code) is
<a href=TestLibs.zip>here</a>; see also its <a href=taufer.txt>README</a>.
<p>
Start by creating a new FORTRAN project.
Add all the FORTRAN specific files,
then add all the files needed for the BOINC library (e.g. boinc_api.C).
Make sure that BOINC and the FORTRAN files are compiled
using the same type of standard libraries.
i.e. if the BOINC is compiled with the debug multithreaded DLL libraries,
make sure the FORTRAN files are compiled with the DLL setting.

<p>
For every BOINC function you want to call from fortran you must add an
interface and subroutine:
<pre>
INTERFACE
  SUBROUTINE boinc_finish(status)
  END SUBROUTINE boinc_finish
END INTERFACE
</pre>

<p>
Remember to declare the type of arguments.
INTEGER status

<p>
You must then tell the compiler that the function
you are interfacing is a C routine.
You do this by adding the statement:
<pre>
 !DEC$ ATTRIBUTES C :: boinc_finish
</pre>
Because BOINC is compiled as C++ files
the FORTRAN compiler will not be able to find
the standard function name in the object file,
you therefore have to add an alias for
the function giving the real function name:
<pre>
 !DEC$ ATTRIBUTES ALIAS : '?boinc_finish@@YAHH@Z' :: boinc__finish
</pre>
<p>
This function name can be found in the object file.
Go to your compile directory and run dumpbin.

<pre>
c:\fortranproject\Release>dumpbin /symbols boinc_api.obj
</pre>

this will give you a list of symbols, where you can find the real functionname.

<p>
The interface will end up looking like this:

<pre>
INTERFACE
  SUBROUTINE boinc_finish(status)
    !DEC$ ATTRIBUTES C :: boinc_finish
    !DEC$ ATTRIBUTES ALIAS : '?boinc_finish@@YAHH@Z' :: boinc__finish
    INTEGER status
  END SUBROUTINE boinc_finish
END INTERFACE

</pre>
You can now call the BOINC function in FORTRAN.
<pre>
call boinc_finish(0)
</pre>

<?php
page_tail();

?>
