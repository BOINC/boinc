<?php
require_once("docutil.php");

page_head("Web Remote Procedure Calls (RPCs)");

echo "
<p>
BOINC projects export a number of Web RPCs
that can be used to create, query and update
accounts and host records.
These can be used for
<a href=acct_mgt.php>account management systems</a> and
credit statistics web sites.
<p>
BOINC's RPC mechanisms have the following conventions:
<ul>
<li> Each RPC is an HTTP GET transaction.
<li> The input is the GET arguments, i.e. a string of the form
".html_text("
param1=val1&param2=val2&...&paramn=valn
")."
where param1 ... paramN are the parameter names,
and val1 ... valn are the values.
<li>
The output is XML.
</ul>

<h3>Create account</h3>
";

list_start();
list_item("URL", "project_url/create_account.php");
list_item(
    "input",
        "email_addr: email address
        <br>passwd_hash: the MD5 hash of the concatenation
        of the user's password and the email address.
        <br>
        user_name: the user name
");
list_item(
    "output",
    html_text("<account_out>
    [ <error_num>N</error_num> ]
    [ <authenticator>XXX</authenticator> ]
</account_out>
    ")
);
list_item(
    "action",
        "If the project already has an account with that email address,
        and a different password, it returns an error.
        If an account with that email address exists
        and has the same password, it returns the authenticator.
        Otherwise the project creates an account
        and returns the authenticator.
");
list_end();

echo "
<h3>Look up account</h3>
";
list_start();
list_item("URL", "project_url/lookup_account.php");
list_item(
    "input",
        "email_addr: email address
        <br>passwd_hash: the MD5 hash of the concatenation
        of the user's password and the email address.
");
list_item(
    "output",
    html_text("<account_out>
    [ <error_num>N</error_num> ]
    [ <authenticator>XXX</authenticator> ]
</account_out>
    ")
);
list_item(
    "action",
    "If an account with the given email address and password hash exists,
    return its account key."
);

list_end();
echo "
<h3>Get account info</h3>
";

list_start();
list_item("URL", "project_url/am_get_info.php");
list_item("input", "account_key");
list_item("output",
    html_text("<am_get_info_reply>
    <success/>
    <id>ID</id>
    <name>NAME</name>
    <country>COUNTRY</country>
    <postal_code>POSTAL_CODE</postal_code>
    <global_prefs>
        GLOBAL_PREFS
    </global_prefs>
    <project_prefs>
        PROJECT_PREFS
    </project_prefs>
    <url>URL</url>
    <send_email>SEND_EMAIL</send_email>
    <show_hosts>SHOW_HOSTS</show_hosts>
    <teamid>N</teamid>
    <venue>X</venue>
</am_get_info_reply>

or

<am_get_info_reply>
    <error>MSG</error>
</am_get_info_reply>
    ")
);
list_item("action", "returns data associated with the given account");
list_end();
echo "
<h3>Set account info</h3>
";
list_start();
list_item("URL", "project_url/am_set_info.php");
list_item("input",
    "account_key
    <br>[ name ]
    <br>[ country ]
    <br>[ postal_code ]
    <br>[ global_prefs ]
    <br>[ project_prefs ]
    <br>[ url ]
    <br>[ send_email ]
    <br>[ show_hosts ]
    <br>[ teamid ]  <i>zero means quit current team, if any</i>
    <br>[ venue ]
    "
);
list_item("output",
    html_text("<am_set_info_reply>
    [ <error>MSG</error> ]
    [ <success/> ]
</am_set_info_reply>")
);
list_item("action",
    "updates one or more attributes of the given account"
);

list_end();

echo "
<h3>Set host info</h3>
";
list_start();
list_item("URL", "project_url/am_set_host_info.php");
list_item("input",
    "account_key
    <br>hostid
    <br>venue
    "
);
list_item("output",
    html_text("<am_set_host_info_reply>
    [ <error>MSG</error> ]
    [ <success/> ]
</am_set_host_info_reply>")
);
list_item("action",
    "Updates the host's venue"
);
list_end();
echo "
<h3>Get account/host credit information</h3>
";
list_start();
list_item("URL",
    "project/show_user.php?userid=X&format=xml or
    project/show_user.php?auth=X&format=xml"
);
list_item("input",
    "user ID or account key"
);
list_item("output",
html_text("<user>
    <id>123</id>
    <cpid>fe0b2753a355b17864ec061eb1b9e8de</cpid>
    <create_time>918948493</create_time>
    <name>Joe Smith</name>
    <country>United States</country>
    <total_credit>3806.869739</total_credit>
    <expavg_credit>200</expavg_credit>
    <expavg_time>1110833427.64028</expavg_time>
    <teamid>114322</teamid>
    <url>example.com</url>
    <has_profile>1</has_profile>
</user>

or

<user>
    <id>123</id>
    <cpid>fe0b2753a355b17864ec061eb1b9e8de</cpid>
    <create_time>1101918807</create_time>
    <name>David</name>
    <country>United States</country>
    <total_credit>0.293197</total_credit>
    <expavg_credit>0.000883</expavg_credit>
    <expavg_time>1116963330.83107</expavg_time>
    <teamid>0</teamid>
    <url>example.com</url>
    <has_profile>1</has_profile>
    <host>
        <id>123</id>
        <host_cpid>fe0b2753a355b17864ec061eb1b9e8de</host_cpid>
        <total_credit>0</total_credit>
        <expavg_credit>0</expavg_credit>
        <expavg_time>0</expavg_time>
        <domain_name>Sorabji</domain_name>
        <p_ncpus>1</p_ncpus>
        <p_vendor>Mobile Intel(R) Pentium(R) 4 - M CPU 2.20GHz</p_vendor>
        <p_model>Pentium</p_model>
        <p_fpops>330806175.78458</p_fpops>
        <p_iops>409200165.535107</p_iops>
        <os_name>Microsoft Windows XP</os_name>
        <os_version>Professional Edition, Service Pack 2, (05.01.2600.00)</os_version>
    </host>
    ...
</user>
")
);
list_item("action",
    "Returns info about an account.
    If called with the account key,
    returns a list of hosts associated with the account."
);
list_end();


page_tail();
?>
