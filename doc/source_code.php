<?php
require_once("docutil.php");

page_head("Getting source code");
echo "

<h2>CVS branches</h2>
<p>
There are two different versions of the BOINC source code
(maintained as separate CVS projects):
<ul>
<li> <b>boinc</b> is the development version.
The client code may be unstable
and may be incompatible with public BOINC projects.
The server code is almost always stable - use this if you
are setting up a server.
<li> <b>boinc_public</b> is the stable version of the client,
used for making bug-fix releases.
</ul>
<h2>Source code</h2>
You can get the BOINC source code in several ways:
<ul>
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
</pre>
<li>
Browse the CVS repository via a
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>web-based interface</a>.
<li>
<a href=source/>Download</a> a tarball or .zip file
(generated nightly).
</ul>
<p>
Source code for a typical BOINC application,
SETI@home, is <a href=http://setiweb.ssl.berkeley.edu/sah/sah_porting.php>here</a>.

";

page_tail();
?>
