<? // -*- html -*-
require_once("docutil.php");
page_head("BOINC: Software prerequisites and corequisites");
?>

BOINC depends on various software to build, test, and run.

<h1>Unix software</h1>

<h2>Development tools</h2>
Required for compiling:
<ul>
  <li><b>gcc</b> 2.95 or 3.0-3.3
  <li><b>make</b>, possibly GNU make
</ul>
<ul>
  <li>Other standard unix tools
</ul>

Optional, required only if you change <code>*/Makefile.am</code>:
<ul>
  <li><b>automake</b> 1.7+
  <li><b>autoconf</b> 2.5+
</ul>


<h1>Windows client software</h1>
<h2>Development tools</h2>
Required for compiling:
<ul>
  <li><b>Microsoft Visual C</b> 6.0
</ul>

<h1>Macintosh client software</h1>
<h2>Development tools</h2>
Required for compiling:
<ul>
  <li><b>?</b>
</ul>


<? page_tail(); ?>
