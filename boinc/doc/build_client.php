<?
require_once("docutil.php");
page_head("Building the BOINC core client");
echo "
<p>
It may not be necessary to build the core client;
you can get executables for many platforms at
<a href=http://boinc.berkeley.edu>http://boinc.berkeley.edu</a>.
<p>
See the <a href=software.php>Software Prerequisites</a>.

<h3>Unix, Mac OS/X</h3>
<p>
<pre>
   cd boinc/client
   make
</pre>
The final target is <code>boinc/client/boinc_VERSION_PLATFORM.gz</code>.

<h3>Windows</h3>

<p>
Open boinc.dsw (MSVC6) or boinc.sln (MSVC7).
Build either the Release or Debug version.
This should also build libraries and screensaver.

";
   page_tail();
?>
