<?php
require_once("docutil.php");
page_head("System requirements");
echo "
The BOINC core client is available for the following platforms:
<ul>
<li> Windows (95 and up)
<li> Linux (on X86 and perhaps others)
<li> Solaris/SPARC
<li> Mac OS X
</ul>
<p>
There are no specific hardware requirements
(CPU speed, RAM, disk space, etc.).
However, these factors may limit the amount or type
of work that is sent to your computer.
Each 'work unit' has minimum RAM and disk requirements,
and a deadline for completion of its computation.
A BOINC project won't send a work unit to a computer that can't handle it.
<p>
If software is not available for your computer,
you may still be able to participate in BOINC projects if you are able to
<a href=anonymous_platform.php>compile the software yourself</a>.
";
page_tail();
?>
