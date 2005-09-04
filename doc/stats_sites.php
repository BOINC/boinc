<?php

require_once("docutil.php");

page_head("Web sites for BOINC statistics");
echo "
<p>
The following web sites show statistics for one or more BOINC projects:
";
stats_sites();
echo "
These sites use XML-format data exported by BOINC projects.
The format is described
<a href=http://boinc.berkeley.edu/db_dump.php>here</a>.
If you are interested in running your own site or
participating in the development efforts,
please contact the people listed above.

<h2>Statistics signature images</h2>
<p>
The following sites offer dynamically-generated
images showing your statistics in BOINC projects.
Use these in your email or message-board signature.
<ul>
<li> http://www.bigbee.be/comp/boinc/signature.php?id=9
</ul>
";

page_tail();

?>
