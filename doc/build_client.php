<?php
require_once("docutil.php");
page_head("Building the BOINC core client");
echo "
<p>
It may not be necessary to build the core client;
you can get executables for many platforms at
<a href=http://boinc.berkeley.edu>http://boinc.berkeley.edu</a>.
<p>
See the <a href=http://boinc.berkeley.edu/trac/wiki/SoftwarePrereqsUnix>Software Prerequisites</a>.

<h3>Unix, Mac OS/X</h3>
<p>
If you have MySQL installed, you can just do:
<pre>
   cd boinc
   configure
   make
</pre>
This will build <code>boinc/client/boinc_VERSION_PLATFORM</code>.

<p>
If you don't have MySQL installed,
that configure step will fail.
You can use the following trick (thanks to Eric Myers for this):

<p>
The idea is to trick the configure script into running a fake
mysql_config script.  An easy way to do that is:

<pre>
  $  export MYSQL_CONFIG=true
</pre>

or the setenv equivalent for tcsh.
This runs /bin/true or similar to
configure mysql, but ./configure does not fail.   

<p>
Of course the server build fails when you say `make` when it tries to
build anything requiring mysql, but if you say `make -k` it will still
build the client and apps, lib and api.
You can also cd to client
or apps and say `make` there and that will work
(once lib and api are built). 

<h3>Windows</h3>

<p>
Open boinc.dsw (MSVC6) or boinc.sln (MSVC7).
Build either the Release or Debug version.
This should also build libraries and screensaver.

";
   page_tail();
?>
