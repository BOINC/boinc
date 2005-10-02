<?php
require_once("docutil.php");

page_head("Getting source code");
echo "

<h2>CVS branches</h2>
<p>
The BOINC source code is maintained in CVS.
The trunk is the development version,
which may not compile or run.
Other versions are tagged as follows:
<dl>
<dt>
stable
<dd>
The latest publicly-released version,
generally well-tested.
<dt>
staging
<dd>
The version currently being alpha-tested
(so at least it compiles).
<dt>
boinc_core_release_x_y_z
<dd>
The source code for version x.y.z.
</dl>

<h2>Source code</h2>
You can get the BOINC source code in two ways:
<ul>
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
</pre>
to get the development version, or
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout -r stable boinc
</pre>
to get the stable version.
<p>
<li>
Browse the CVS repository via a
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>web-based interface</a>.
</ul>
<p>
Source code for a typical BOINC application,
SETI@home, is <a href=http://setiweb.ssl.berkeley.edu/sah/sah_porting.php>here</a>.

";

page_tail();
?>
