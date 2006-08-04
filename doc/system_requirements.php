<?php

require_once("docutil.php");
page_head("System requirements");

echo "
<b>Your computer must satisfy the following requirements to run BOINC.
BOINC-based projects may have additional requirements.</b>

<hr>
<h2>Windows<h2>
<h4>Operating systems</h4>
<ul>
<li> Windows 98 and later
</ul>
<h4>Minimum hardware</h4>
<ul>
<li> Pentium 233 MHz (Recommended: Pentium 500 MHz or greater)
<li> 64 MB RAM (Recommended: 128 MB RAM or greater)
<li> 20 MB disk space
</ul>

<hr>
<h2>Macintosh</h2>
<h4>Operating systems</h4>
<ul>
<li> Mac OS X 10.3 and later
</ul>
<h4>Minimum hardware</h4>
<ul>
<li> Macintosh computer with an Intel x86
or PowerPC G3, G4, or G5 processor
<li> 128 MB RAM (Recommended: 256 MB RAM or greater)
<li> 200 MB disk space
</ul>

<hr>
<h2>Linux</h2>
<h4>Operating systems</h4>
<ul>
<li> Linux kernel 2.2.14 or higher
<li> glibc 2.3.2 or higher
<li> XFree86-3.3.6 or higher
<li> gtk+2.0 or higher
</ul>
<h4>Minimum hardware</h4>
<ul>
<li> Pentium 500 MHz or greater
<li> 64 MB RAM
<li> 50 MB disk space
</ul>
<hr>
";

page_tail();

?>
