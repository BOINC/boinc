<?php
require_once("docutil.php");

page_head("Web Remote Procedure Calls (RPCs)");

echo "
<h3>Contents</h3>
<ul>
<li> <a href=#overview>Overview</a>
<li> <a href=#project_config#>Project configuration</a>
<li> <a href=#create_account>Create account</a>
<li> <a href=#lookup_account>Lookup account</a>
<li> <a href=#am_get_info>Get account info</a>
<li> <a href=#am_set_info>Set account info</a>
<li> <a href=#am_set_host_info>Set host info</a>
<li> <a href=#show_user>Get account/host credit info</a>
<li> <a href=#create_team>Create team</a>
<li> <a href=#team_lookup>Lookup teams by name</a>
<li> <a href=#team_lookup_id>Lookup team by ID</a>
<li> <a href=#team_email_list>Get team member list</a>
<li> <a href=#edit_forum_preferences_action>Set forum preferences</a>
<li> <a href=#forum_get_user_posts>Get last user's posts from the forum</a>
<li> <a href=#forum_get_user_threads>Get last user's threads from the forum</a>
</ul>

<a name=overview></a>
<h3>Overview</h3>
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
Parameter values must be URL-encoded
(for example, using PHP's <code>urlencode</code> function).
<li>
If an RPC fails, the returned XML document is
".html_text("<error>
    <error_num>N</error_num>
    <error_string>xxx</error_string>
</error>")."
where N is a BOINC error number and xxx is a textual description.
BOINC error numbers are in lib/error_numbers.h; common errors are:
";
list_start();
list_item("-1", "Generic error (error_string may have more info)");
list_item("-112", "Invalid XML (e.g., the preferences passed to am_set_info.php are invalid)");
list_item("-136", "Item not found in database
    (bad ID of any sort, or ID refers to an item not owned by the caller)");
list_item("-137", "Name is not unique (Can't create account because
    email address already in use,
    or can't create team because name is in use)");
list_item("-138", "Can't access database (treat same as -183)");
list_item("-161", "Item not found (deprecated; treat same as -136)");
list_item("-183", "Project is temporarily down");
list_item("-205", "Email address has invalid syntax");
list_item("-206", "Wrong password");
list_item("-207", "Non-unique email address (treat same as -137)");
list_item("-208", "Account creation disabled");
list_end();
echo "
<li>
The output is XML.
<li>
If the project's <a href=project_config.php>get_project_config.php</a> file
includes a <code>&lt;rpc_prefix&gt;</code> element,
its content should be used as the URL prefix;
otherwise use the project's master URL.

</ul>


<a name=project_config></a>
<h3>Project configuration</h3>
Each BOINC project exports some configuration items relevant
to attaching and creating accounts.
This RPC is documented
<a href=project_config.php>here</a>.

<a name=create_account></a>
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
    <authenticator>XXX</authenticator>
</account_out>")
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
<a name=lookup_account></a>
<h3>Look up account</h3>
";
list_start();
list_item("URL", "project_url/lookup_account.php");
list_item(
    "input",
        "email_addr: email address
        <br>
        [ passwd_hash ]: the MD5 hash of the concatenation
        of the user's password and the email address.
");
list_item(
    "output",
    html_text("<account_out>
    [ <authenticator>XXX</authenticator> ]
</account_out>
    ")
);
list_item(
    "action",
    "If an account with the given email address doesn't exist,
    return an error.
    If passwd_hash is given and is correct,
    return the account key."
);

list_end();
echo "
<a name=am_get_info></a>
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
    [ <teamfounder/> ]
    <venue>X</venue>
</am_get_info_reply>
    ")
);
list_item("action", "returns data associated with the given account");
list_end();
echo "
<a name=am_set_info></a>
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
    <br>[ email_addr ]
    <br>[ password_hash ]
    "
);
list_item("output",
    html_text("<am_set_info_reply>
    <success/>
</am_set_info_reply>")
);
list_item("action",
    "Updates one or more attributes of the given account.
    The password hash is MD5(password+lower_case(email_addr)).
    If email address is changed,
    you must also change the password hash."
);

list_end();

echo "
<a name=am_set_host_info></a>
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
    <success/>
</am_set_host_info_reply>")
);
list_item("action",
    "Updates the host's venue"
);
list_end();
echo "
<a name=show_user></a>
<h3>Get account/host credit information</h3>
";
list_start();
list_item("URL",
    "project/show_user.php?userid=X&format=xml or
    project/show_user.php?auth=X&format=xml"
);
list_item("input",
    "id (user ID) or auth (account key)"
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
        <create_time>1287339128</create_time>
        <rpc_seqno>123</rpc_seqno>
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
echo "
<a name=create_team></a>
<h3>Create team</h3>
";
list_start();
list_item("URL",
    "project/create_team.php"
);
list_item("input",
    "account_key: identifies team founder
<br>name: name of team
<br>url: team URL (optional)
<br>type: one of the types listed <a href=team_types.php>here</a>.
<br>name_html: team name, HTML (optional)
<br>description: text describing team (optional)
<br>country: team country (optional; if present,
must be one of the countries listed <a href=countries.php>here</a>.
");
list_item("output",
    html_text("<create_team_reply>
    <success/>
    <teamid>N</teamid>
</create_team_reply>
"));
list_item("action",
    "Creates a team"
);
list_end();
echo "
<a name=team_lookup></a>
<h3>Look up teams by name</h3>
";
list_start();
list_item("URL",
    "project/team_lookup.php?team_name=string&format=xml"
);
list_item("input",
    "Substring of team name"
);
list_item("output",
    html_text("<teams>
    <team>
        <id>5</id>
        <name>BOINC@AUSTRALIA</name>
        <country>Australia</country>
    </team>
    <team>
        <id>9</id>
        <name>BOINC Synergy</name>
        <country>International</country>
    </team>
    <team>
        <id>16</id>
        <name>BOINC.BE</name>
        <country>Belgium</country>
    </team>
    <team>
        <id>23</id>
        <name>HispaSeti & BOINC</name>
        <country>International</country>
    </team>
</teams
")
);
list_item("action",
    "Teams with names matching *string* will be returned.
    A maximum of 100 teams will be returned."
);
list_end();

echo "
<a name=team_lookup_id></a>
<h3>Look up team by ID</h3>
";
list_start();
list_item("URL",
    "project/team_lookup.php?team_id=N"
);
list_item("input",
    "Team ID"
);
list_item("output",
    html_text("<team>
    <id>5</id>
    <name>BOINC@AUSTRALIA</name>
    <country>Australia</country>
</team>
")
);
list_item("action",
    "Show info on team with the given ID."
);
list_end();

echo "
<a name=team_email_list></a>
<h3>Get team member list</h3>
";
list_start();
list_item('URL',
    'project/team_email_list.php?teamid=X&account_key=Y&xml=1'
);
list_item("input",
    'teamid: database ID of team
    <br>account_key: account key of team founder (optional)'
);
list_item("output",
    html_text("<users>
    <user>
        <id>1</id>
        <email_addr>pdq@usndathoople.edu</email_addr>
        <cpid>232f381c79336f0bd8df02bbce2f2217</cpid>
        <create_time>1076897105</create_time>
        <name>David</name>
        <country>United States</country>
        <total_credit>9.907264</total_credit>
        <expavg_credit>0.023264</expavg_credit>
        <expavg_time>1142628426.48937</expavg_time>
        <url>usndathoople.edu/~pdq</url>
        <has_profile>1</has_profile>
    </user>
    [ ... ]
</users>")
);
list_item('action',
    'Show list of team members.
    If account key is that of team founder, show email addresses.'
);
list_end();

echo "
<a name=edit_forum_preferences_action></a>
<h3>Set forum preferences</h3>
";

list_start();
list_item('URL',
    'project/edit_forum_preferences_action.php
    <br>
    NOTE: this uses POST, not GET
    '
);
list_item('input',
    'account_key
    <br>avatar_url
    <br>avatar_select
    <br>forum_images_as_links
    <br>forum_link_externally
    <br>forum_hide_avatars
    <br>forum_hide_signatures
    <br>forum_jump_to_unread
    <br>forum_ignore_sticky_posts
    <br>forum_low_rating_threshold
    <br>forum_high_rating_threshold
    <br>forum_minimum_wrap_poastcound
    <br>forum_display_wrap_poastcound
    <br>signature_enabled
    <br>signature
    <br>forum_sort
    <br>thread_sort
    <br>faq_sort
    <br>answer_sort
    <br>forum_filter_user
    <br>[ removeID ... ]
'
);
list_item('output',
    'A lot of HTML (not XML)'
);
list_item('action',
    'Update user\'s forum preferences'
);
list_end();


echo "
<a name=\"forum_get_user_posts\"></a>
<h3>Get last user's posts from the forum</h3>
";

list_start();
list_item('URL',
    'project/forum_get_data.php?method=user_posts&amp;userid=N&amp;count=N
    '
);
list_item('input',
    'userid: numeric user ID in the database
    <br />count <i>(optional)</i>: number of entries to return. Maximum 50, default 10.
'
);
list_item('output',
    html_text("<rpc_response>
    <count>1</count>
    <posts>
        <post>
        <id>4157</id>
        <threadid>76</threadid>
        <timestamp>1162847905</timestamp>
        <content><![CDATA[Example post content (truncated to 100 characters).]]></content>
        </post>
        [ ... ]
    </posts>
</rpc_response>")
);
list_item('action',
    'Get last user\'s posts from the forum.'
);
list_end();


echo "
<a name=\"forum_get_user_threads\"></a>
<h3>Get last user's threads from the forum</h3>
";

list_start();
list_item('URL',
    'project/forum_get_data.php?method=user_threads&amp;userid=N&amp;count=N
    '
);
list_item('input',
    'userid: numeric user ID in the database
    <br />count <i>(optional)</i>: number of entries to return. Maximum 50, default 10.
'
);
list_item('output',
    html_text("<rpc_response>
    <count>1</count>
    <threads>
        <thread>
            <id>356</id>
            <forumid>2</forumid>
            <replies>11</replies>
            <views>612</views>
            <timestamp>1159062318</timestamp>
            <title><![CDATA[Example forum thread title]]></title>
        </thread>
        [...]
    </threads>
</rpc_response>")
);
list_item('action',
    'Get last user\'s threads from the forums.'
);
list_end();

page_tail();
?>
