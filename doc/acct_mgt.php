<?php
require_once("docutil.php");

page_head("Account management systems");

echo "
<p>
To create an account with BOINC projects, a participant must:
<ul>
<li> Locate BOINC project web sites, e.g. using Google.
<li> Read the web sites, and decide which to join;
<li> Download and install the BOINC client software.
</ul>
and then for each selected project:
<ul>
<li> Go through the Attach to Project wizard.
</ul>

<p>
If the participant chooses N projects,
there are N web sites to visit and N Wizards to complete.
This is tedious if there are lots of projects.

<p>
This document describes BOINC's support for <b>account management systems</b>,
which streamline the process of finding and joining BOINC projects.
A typical account management system is implemented as a web site.
The participant experience is:
<ul>
<li> Visit the account manager site,
set up a 'meta-account' (name, email, password),
browse a list of projects, and click checkboxes to select projects.
<li> Download and install the BOINC client software from the account manager.
<li> Enter the meta-account name and password in a BOINC client dialog.
</ul>
This requires many fewer interactions than the manual approach.

<h2>Implementation</h2>
<p>
An account management system works as follows:
<br>
<img src=acct_mgt2.png>
<br>

<ol>
<li> The participant sets up his meta-account and selects projects.
<li> The account manager issues a <b>create account</b> RPC
to each selected project.
<li> 
the participant downloads and installs the BOINC client software
from the account manager.
The install package includes a file
(specific to the account manager)
containing the URL of the account manager.
<li> The BOINC client runs, and asks the participant to enter
the name and password of his meta-account.
<li> The BOINC client does an RPC
to the account manager, obtaining a list of accounts.
It then attaches to these accounts and proceeds.
</ol>

RPCs to create, look up, and modify accounts
are described <a href=web_rpc.php>here</a>.

<h2>Security</h2>
<p>
If hackers break into an account manager server,
they could potentially cause the account manager
to instruct all its clients to attach to malicious a BOINC project
that runs a malicious application.
To prevent this type of attack, the URLs distributed by
an account manager are digitally signed.
Each AM has its own <b>signing key</b> pair.
The public key is distributed with the AM's configuration file
and in all RPC replies.
The private key should be stored only on a physically secure,
non-connected host that is used to sign URLs.

<p>
To sign URLs, compile <a href=key_setup.php>crypt_prog</a>,
BOINC's encryption utility program.
(Instructions for downloading and compiling code
are <a href=server.php>here</a>.)
Generate a key pair and generate signatures for your URLs.
At some point you'll need to commit to a permanent key pair,
at which point you should move the private key to
the signing machine (disconnected).
Make a copy or two on CD-ROM also, and/or print it out on paper;
keep these in a safe place.
Delete all other copies of the private key.

<h2>Farm managers</h2>
<p>
The AM mechanism can also be used to implement systems for
configuring and controlling BOINC on large clusters.
We call such systems <b>farm managers</b>.
Farm managers may want to provide fine-grained control over clients,
e.g. the ability to suspend/resume results.
This can be done using GUI RPCs (assuming that the farm manager able
to contact clients via HTTP on the GUI RPC port).
However, the farm manager must learn the GUI RPC port and password
for each client.
To support this, the AM configuration file (see below) can specify
that the GUI RPC port and password are to be included in each AM RPC request.
<p>
If a farm manager uses GUI RPC to attach/detach projects,
it should not use the AM mechanism for this purpose.
I.e., its AM RPC replies should not list any projects.
The function of the AM mechanism, in this case,
is to allow sysadmins to set up new clients by copying files.
The AM mechanism takes care of registering new clients centrally.

<h2>Core client functionality</h2>
<p>
The BOINC core client uses the following files to
keep track of account manager information.
<dl>
<dt>
<b>acct_mgr_url.xml</b>
<dd>
This file identifies the account manager.
It is typically bundled with the BOINC client in
an installer package.
Its format is:
".html_text("
<acct_mgr>
    <name>Name of BOINC account management system</name>
    <url>http://acctmgr.com/</url>
    [ <send_gui_rpc_info/> ]
    <signing_key>
1024
ae843acebd4c7250b0fa575d14971b17a56a386a6bb1733d98f4b00460c26159
c8b3217e6cdff938ec0454330c70553fbe3d1f0d0184d8c628db2e093121ee98
8ddbda6e8991879317afccab41f84e9de4903a656f4d3f3e4e7dbc0af9362a05
6ece5ff401a380f3a1d1254d477f7bc84fdcebcca6cb035e776452d3d6d21471
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000010001
.
    </signing_key>
</acct_mgr>
")."

<p>
The URL is that of the account manager's web site.
<p>
If the &lt;send_gui_rpc_info/&gt; tag is present,
account manager RPCs will include
the client's GUI RPC port and password hash (see below).


<dt>
<b>acct_mgr_login.xml</b>
<dd>
This file contains meta-account information.
Its format is:
".html_text("
<acct_mgr_login>
   <login>name</login>
   <password_hash>xxx</password_hash>
</acct_mgr_login>
")."
</dl>
<p>
The password is stored as MD5(password_lowercase(login)).
<p>
If the core client finds acct_mgr_url.xml but not acct_mgr_login.xml,
it prompts for a name and password,
stores them in acct_mgr_login.xml,
and makes an account manager RPC.
The core client offers menu items for making an account manager RPC,
and for changing the name/password.

<h2>Account manager RPCs</h2>
<p>
An account manager must provide a
<a href=project_config.php>get_project_config.php</a> file
containing its name and minimum password length,
and containing a <code> &lt;account_manager/&gt;</code> tag.
<p>
In addition, an account manager must provide the following RPC,
which uses an HTTP POST request.
";

list_start();
list_item("URL", "<b>BASE_URL/rpc.php</b>, where BASE_URL is the URL
        of the account manager web site.");
list_item("input", html_text("
<acct_mgr_request>
    <name>John</name>
    <password_hash>xxx</password_hash>
    <host_cpid>b11ddc5f36c9a86ff093c96e6930646a</host_cpid>
    <previous_host_cpid>b11ddc5f36c9a86ff093c96e6930646a</previous_host_cpid>
    <domain_name>host.domain</domain_name>
    <client_version>5.3.2</client_version>
    <run_mode>auto</run_mode>
    <global_preferences>
        <source_project>http://a.b.c</source_project>
        <source_scheduler>http://a.b.c</source_scheduler>
        <mod_time>1144105331</mod_time>
        ... [ global preferences ]
    </global_preferences>
    <project>
       <url>http://setiathome.berkeley.edu/</url>
       <project_name>SETI@home</project_name>
       <suspended_via_gui>0</suspended_via_gui>
       <account_key>397d250e02ec02be8141b8d109d5ec73e5</account_key>
       <hostid>NNN</hostid>
       [ <attached_via_acct_mgr/> ]
    </project>
    [ ... other projects ]
    [ <gui_rpc_port>N</gui_rpc_port> ]
    [ <gui_rpc_password>xxxx</gui_rpc_password> ]
    ...
</acct_mgr_request>
")
);
list_item("output",
    html_text("<acct_mgr_reply>
    <name>Account Manager Name</name>
    <signing_key>
1024
ae843acebd4c7250b0fa575d14971b17a56a386a6bb1733d98f4b00460c26159
c8b3217e6cdff938ec0454330c70553fbe3d1f0d0184d8c628db2e093121ee98
8ddbda6e8991879317afccab41f84e9de4903a656f4d3f3e4e7dbc0af9362a05
6ece5ff401a380f3a1d1254d477f7bc84fdcebcca6cb035e776452d3d6d21471
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000000000000000000000000000000000000000010001
.
    </signing_key>
    [ <message>this is a message</message> ]
    [ ... ]
    [ <error>MSG</error> ]
    [ <repeat_sec>xxx</repeat_sec> ]
    [
    <global_preferences>
        [ <source_project>http://a.b.c</source_project> ]
        [ <source_scheduler>http://a.b.c</source_scheduler> ]
        <mod_time>1144105331</mod_time>
        ... [ global preferences ]
    </global_preferences>
    ]
    [ <host_venue>venue</host_venue> ]
    [ 
    <account>
       <url>URL</url>
       <url_signature>
397d250e02ec02be8141b8d196a118d909d5ec73e592ed50f9d0ad1ce5bf87de
e37f48079db76128b20f913a04e911489330a7cab8c346177f1682d236bc7201
42b32665d0d83474bf12aebd97b2bb9a4c4461fa3f0b49bbd40ecfa16715ced7
f72103eb0995be77cac54f253c0ba639a814d3293646ae11894e9d1367a98790
.
       </url_signature>
       <authenticator>KEY</authenticator>
       [ <update>0|1</update> ]
       [ <detach>0|1</detach> ]
       [ <dont_request_more_work>0|1</dont_request_more_work> ]
       [ <detach_when_done>0|1</detach_when_done> ]
       [ <resource_share>X</resource_share> ]
    </account>
    ...
    ]
</acct_mgr_reply>")
);
list_item("action",
    "Returns a list of the accounts associated with this meta-account.
    The arguments are:
    <dl>
    <dt>password_hash
    <dd>
    the account password, hashed as MD5(password_lowercase(name)).
    <dt>host_cpid
    <dd>
    Identifies the host.
    <dt>previous_host_cpid
    <dd>
    The host identifier passed in the previous RPC.
    Host identifiers can change occasionally.
    This lets the account manager maintain host identity.
    <dt> domain_name
    <dd> The host's domain name.
    <dt> run mode
    <dd> The current mode (always/auto/never).
    <dt> gui_rpc_port, gui_rpc_password
    <dd> GUI RPC information.
        Included only if the &lt;send_gui_rpc_info&gt; element
        is included in the AM URL file (see above).
    <dt> global_preferences
    <dd> The current global preferences.
    <dt> venue
    <dd> The host venue
    </dl>
    In addition, a list of projects and their suspended flags is included.
    <p>
    The return values are:
    <dl>
    <dt>repeat_sec
    <dd>
    A time interval after which another RPC should be done.
    <dt> signing_key
    <dd>
    The public key used to sign URLs, in an encoded notation.
    Use the BOINC <a href=key_setup.php>crypt_prog</a> program to generate this.
    <dt> message
    <dd> A message to be displayed to the user.
    <dt> global_preferences
    <dd> New global preferences.  Included only if these are
        newer than those in the request message.
    </dl>
    For each account, the following items are returned:
    <dl>
    <dt>url
    <dd>The project URL
    <dt>url_signature
    <dd>A signature for the URL.
    Use the BOINC <a href=key_setup.php>crypt_prog</a> program to generate this.
    <dt>authenticator
    <dd>The account's authenticator.
    <dt>detach
    <dd>If nonzero, the client should detach this project.
    <dt>update
    <dd>If nonzero, the client should contact this project
        to get new global preferences.
    <dt>dont_request_more_work
    <dd>If nonzero, don't request any more work from this project.
    <dt>detach_when_done
    <dd>If nonzero, detach from this project when all work is completed
    <dt>resource_share
    <dd>Specifies a resource share for this project.
        If present, this overrides the resource share reported by the project.
        Thus, account managers can provide per-host control of resource share.
    </dl>
    NOTE: the XML must be as above, with the &lt;url>
    and &lt;authenticator> elements on a single line,
    and the &lt;account> and &lt;/account> tags
    on separate lines."
);
list_end();

echo "
<h2>Host identification</h2>
<p>
BOINC uses two ID spaces for hosts:
<ul>
<li> <b>Cross-project ID (CPID)</b>.
This is used in the statistics data exported by projects.
The 'authoritative' CPID is stored on the host itself.
A host's CPID changes whenever it attaches to a new project.
CPID is propagated to projects in scheduler RPCs.
If a host is attached to several projects,
its CPID (as reported by those projects) may be
inconsistent for several days.
Because of these properties, CPID is not useful as a primary host identifier.


<li> <b>Project database ID (DBID)</b>.
This is a project-specific integer identifier.
On a given host, it changes only in the rare case where
a user copies state files from one computer to another;
in that case, one of the two computers is assigned a new ID
after its next scheduler RPC.

</ul>

An account manager RPC request includes the host's CPID,
and its DBID on all projects to which it's attached (see above).

<p>
A suggested method of maintaining a user's hosts is as follows:

<ul>
<li> Maintain a host table with columns
    <ul>
    <li> user ID
    <li> project ID
    <li> DBID
    <li> CPID
    </ul>
<li> On each account manager RPC,
    for each reported project,
    look up (user ID, project ID, DBID) in
    the host table
    (where 'DBID' is the one passed in the RPC request).
    If no record is found, create one.
    In any case, update the record with the CPID from the RPC request.

<li> To show the user a list of their hosts,
    with current per-project credit:
    do a <a href=web_rpc.php>show_user.php</a>
    RPC to each project to get a list of hosts.
    Update the corresponding records in the host table,
    based on the DBIDs returned by show_user.php.
    Delete any host table entries for this user/project that 
    don't appear in the RPC reply.
    Ignore the CPIDs returned by the show_user RPC
    (they may not be consistent).
    Then do a query on the host table, grouping by CPID.
</ul>
";
page_tail();
?>
