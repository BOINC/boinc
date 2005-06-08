<?php
require_once("docutil.php");
page_head("Source code road map");
echo "
<p>
BOINC tree includes the following directories:
";
list_start();
list_item("RSAEuro",
    "An RSA implementation (included for convenience; not covered
    under the BOINC public license)."
);
list_item("api",
    "The BOINC API (for applications)"
);
list_item("apps",
    "Some test applications."
);
list_item("client",
    "The BOINC core client."
);
list_item("db",
    "The database schema and interface functions."
);
list_item("doc",
    "HTML documentation files."
);
list_item("html/ops",
    "PHP files for the operational web interface."
);
list_item("html/user",
    "PHP files for the participant web interface."
);
list_item("lib",
    "Code that is shared by more than one component
    (core client, scheduling server, etc.)."
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
list_end();
page_tail();
?>
