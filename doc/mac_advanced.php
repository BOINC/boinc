<?php
require_once("docutil.php");
page_head("Advanced Mac installation");
echo "
<ul>
<li> Put the Stuffit Archive anywhere on your Mac
(I suggest the /Applications Folder)
<li> expand it by double-clicking it.
The BOINC application bundle includes the BOINC client embedded inside it.
<li> Run the BOINC application.
It will create a \"BOINC Data\" folder inside your
\"~/Library/Applications Support\" folder if one does not already exist.
<li>
Then just click \"Attach Project\" and it should go from there. 
</ul>
";
page_tail();
?>
