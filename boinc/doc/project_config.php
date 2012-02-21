<?php
require_once("docutil.php");
page_head("Project configuration web RPC");
echo "
Each BOINC project and account manager exports
a few configuration items via a web RPC;
namely, it must provide an XML document at the address
<pre>
http://PROJECT_URL/get_project_config.php
</pre>
This file is created by <a href=make_project.php>make_project</a>,
and in most cases you don't have to change it.

<p>
This document has the structure
".html_text("
<project_config>
    <name>Project name</name>
    [ <min_passwd_length>N</min_passwd_length> ]
    [ <account_manager/> ]
    [ <uses_username/> ]
    [ <account_creation_disabled/> ]
    [ <client_account_creation_disabled/> ]
    [ <rpc_prefix>URL</rpc_prefix> ]
    [ <error_num>N</error_num> ]
    [
      <system_requirements>
        [ <p_fpops>N</p_fpops> ]
        [ <p_iops>N</p_iops> ]
        [ <p_membw>N</p_membw> ]
        [ <m_nbytes>N</m_nbytes> ]
        [ <m_swap>N</m_swap> ]
        [ <d_free>N</d_free> ]
        [ <bwup>N</bwup> ]
        [ <bwdown>N</bwdown> ]
      </system_requirements>
    ]
    [
      <platforms>
        <platform>windows_intelx86</platform>
        <platform>i686-pc-linux-gnu</platform>
        <platform>powerpc-apple-darwin</platform>
        ...
      </platforms>
    ]
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
    "If present, new accounts can be created only via the web
    (not via the client software)."
);
list_item("min_passwd_length",
    "Minimum password length (for new account creation)"
);
list_item("rpc_prefix",
    "Prefix to use for web RPCs, instead of the master URL."
);
list_item("error_num",
    "The project is currently down.  A BOINC error number is returned."
);
list_item("system_requirements",
    "Hardware requirements for participating in this project.
    If a computer doesn't meet these requirements it may not
    get sent any work by the project.
    All requirements are 'net'; e.g. the CPU requirements
    are after factors like <a href=sched.php>on-fraction, active-fraction</a>,
    and resource share have been taken into consideration.
    NOT IMPLEMENTED YET."
);
list_item("platforms",
    "A list of platforms for which the project has application versions.
    NOT IMPLEMENTED YET."
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
