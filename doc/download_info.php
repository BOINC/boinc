<?php
require_once("docutil.php");
page_head("Specialized download pages");
echo "
Some sites may want versions of the BOINC download page
that show only particular versions or platforms.
This can be done by adding parameters to the
http://boinc.berkeley.edu/download.php URL.
";
list_start();
list_item("dev=1", "Show versions that are still under test");
list_item("min_version=x", "Show no versions earlier than x");
list_item("max_version=x", "Show no versions later than x");
list_item("version=x", "Show version x");
list_item("platform=x", "Show versions for platform x (win/mac/linux/solaris)");
list_item("xml=1", "Show results as XML (other options are ignored).");
list_end();
echo "
For example:
<a href=http://boinc.berkeley.edu/download.php?min_version=5.0&dev=1>http://boinc.berkeley.edu/download.php?min_version=5.0&dev=1</a>
shows only versions 5.0 and later, include test versions.
";
page_tail();
?>
