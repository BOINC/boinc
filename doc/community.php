<?
require_once("docutil.php");

page_head("Community and resources");

echo "
<h2>Participants</h2>
<p>
If you have questions about using the BOINC software,
or have a bug to report, please go to the
<a href=http://setiboinc.ssl.berkeley.edu/ap/forum/help_desk.php>Message
board area</a> of the BOINC beta test.

<p>
There are a number of
<a href=links.php>other web sites related to BOINC</a>.

<h2>Getting source code</h2>
<p>
You can get the BOINC source code in several ways:
<ul>
<li>
<a href=source/>download</a> a tarball or .zip file
(generated nightly).
<li>
Browse the CVS repository via a
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>web-based interface</a>.
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
<pre>
</ul>
<h2>Developers and project creators</h2>
<p>
There is a mailing list, boincdev@ssl.berkeley.edu,
for BOINC developers.
To be added to it, please email David Anderson (address below).
We are in the process of automating this list.
<p>
You can browse the BOINC
<a href=http://setiathome.ssl.berkeley.edu/taskbase/database.cgi>bug-tracking database</a>.

<h2>Other</h2>
For other information, contact Dr. David P. Anderson,
the director of the BOINC project, at davea at ssl.berkeley.edu.
If you have problems with the BOINC software for Windows please email
Rom Walton: rwalton at ssl.berkeley.edu.

";

page_tail();
?>
