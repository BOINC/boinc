<?php
require_once("docutil.php");
page_head("Project XML document");
echo "
Each BOINC project and account manager must provide an XML document
at the address
<pre>
http://PROJECT_URL.get_project_config.php
</pre>
This document has the structure
".html_text("
<project_config>
    <name>Project name</name>
    [ <min_passwd_length>N</min_passwd_length> ]
    [ <account_manager/> ]
    [ <uses_username/> ]
    [ <account_creation_disabled/> ]
    [ <client_account_creation_disabled/> ]
    [ <error_num>N</error_num> ]
</project_config>
")."
The elements are:
";
list_start();
list_item("name", "Project name");
list_item("account_manager",
    "If present, this is an account manager, not a BOINC project"
);
list_item("uses_username",
    "If present, this project uses names (rather than email addresses)
    as the primary account identifier"
);
list_item("account_creation_disabled",
    "If present, this project is not allowing creation of new accounts"
);
list_item("client_account_creation_disabled",
    "If present, this project doesn't allow account creation via web RPCs.
    Users must visit the web site to create accounts."
);
list_item("min_passwd_length",
    "Minimum password length (for new account creation)"
);
list_item("error_num",
    "The project is currently down.  A BOINC error number is returned."
);
list_end();
echo "
The BOINC distribution includes a file
<b>html/user/sample_get_project_config.php</b>
that supplies reasonable default values for BOINC projects.
To use this, rename it to <b>get_project_config.php</b>
";
page_tail();
?>
