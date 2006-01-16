<?php
require_once("docutil.php");
page_head("Source code road map");
echo "
<p>
The BOINC source tree includes the following directories:
";
list_start();
list_item("api",
    "The BOINC API (for applications)"
);
list_item("apps",
    "Some test applications."
);
list_item("client",
    "The BOINC core client."
);
list_item("clientgui",
    "The BOINC Manager."
);
list_item("db",
    "The database schema and C++ interface layer."
);
list_item("doc",
    "BOINC documentation (PHP web pages)."
);
list_item("html/ops",
    "PHP files for the operational web interface."
);
list_item("html/user",
    "PHP files for the participant web interface."
);
list_item("html/inc",
    "PHP include files."
);
list_item("html/languages",
    "Translation files."
);
list_item("lib",
    "Code that is shared by more than one component
    (core client, scheduling server, etc.)."
);
list_item("py",
    "Python modules used by tools."
);
list_item("sched",
    "The scheduling server, feeder, and file upload handler."
);
list_item("test",
    "Test scripts."
);
list_item("tools",
    "Operational utility programs."
);
list_item("zip",
    "Compression functions; not used by BOINC,
    but may be useful for applications."
);
list_end();
page_tail();
?>
