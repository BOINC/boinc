<?php

require_once("docutil.php");

include("../html/inc/stats_sites.inc");

page_head("Web sites for BOINC statistics");
echo "
<p>
The following web sites show statistics for one or more BOINC projects:
";
shuffle($stats_sites);
site_list($stats_sites);
echo "
These sites use XML-format data exported by BOINC projects.
The format is described
<a href=http://boinc.berkeley.edu/stats.php>here</a>.
If you are interested in running your own site or
participating in the development efforts,
please contact the people listed above.

<h2>Statistics signature images</h2>
<p>
The following sites offer dynamically-generated
images showing your statistics in BOINC projects.
Use these in your email or message-board signature.
";
shuffle($sig_sites);
site_list($sig_sites);

page_tail();

?>
