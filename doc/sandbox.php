<?php
require_once("docutil.php");
page_head("Sandbox design");

echo "
This document describes a proposed modification to the Unix
(including Linux and Mac OS X) versions of BOINC.
The goal of this change is to 'sandbox' BOINC applications,
i.e. to limit the amount of damage that a malicious
or malfunctioning application can cause.
<p>
In our design, BOINC applications run under a specially-created account
having a minimal set of privileges.
Previously, the applications typically ran as the user who installed BOINC,
and had the full privileges of that account.
";

function prot($user, $group, $perm) {
    return "
    <br>
    &nbsp;&nbsp; user: $user
    <br>
    &nbsp;&nbsp; group: $group
    <br>
    &nbsp;&nbsp; protection: $perm";
}

$pp0775 = prot('boinc_project', 'boinc_project', '0775');
$mp2500 = prot('boinc_master', 'boinc_project', '0500+setgid');
$pp6551 = prot('boinc_project', 'boinc_project', '0551+setuid+setgid');
$mm0550 = prot('boinc_master', 'boinc_master', '0550');
$mm0770 = prot('boinc_master', 'boinc_master', '0770');
$mm0775 = prot('boinc_master', 'boinc_master', '0775');
$mp0775 = prot('boinc_master', 'boinc_project', '0775');
$mm2555 = prot('boinc_master', 'boinc_master', '0555+setgid');
$mm6555 = prot('boinc_master', 'boinc_master', '0555+setuid+setgid');
$ua0555 = prot('(installing user)', 'admin', '0555');

$colors = array('ddddff', 'ccccff', 'bbbbff');

function show_dir($level, $name, $prot, $contents) {
    global $colors;
    $color = $colors[$level];
    $x = "
        <table bgcolor=$color cellpadding=6 cellspacing=0 border=1 width=100%>
        <tr>
            <td valign=top><b>$name</b> <font size=-1>$prot</font></td><td valign=top>
    ";
    for ($i=0; $i<sizeof($contents); $i++) {
        if ($i) $x .= '<br>';
        $c = $contents[$i];
        $x .= $c;
    }
    $x .= "
        </td></tr>
        </table>
    ";
    return $x;
}

function show_file($name, $prot) {
    return "
            $name <font size=-1>$prot</font><br>
    ";
}

echo "

<p>
Our design uses two users and two groups,
both specially created for use by BOINC.
These users and groups are created by the installation process.
<ul>
<li>Group: <b>boinc_master</b>
<li>Group: <b>boinc_project</b>
<li>User: <b>boinc_master</b>
<ul>
<li>Primary group: <b>boinc_master</b>
<li>Supplementary groups: <b>boinc_project</b>
</ul>
<li>User: <b>boinc_project</b>
<ul>
<li>Primary group: <b>boinc_project</b>
<li>Supplementary groups: none
</ul>
</ul>
On Mac OS X, <b>boinc_project</b> and <b>boinc_master</b>
are added to the Supplementary Groups Lists of those other users
who are members of group <b>admin</b>.
This gives admin users full access to all BOINC and project files.
<p>
The following diagram shows user, group and permissions
for the BOINC file and directory tree:
<p>
";

echo
    show_dir(0, 'BOINC data', $mm0775, array(
        show_dir(1, 'projects', $mm0775, array(
            show_dir(2, 'setiathome.berkeley.edu', $mp0775, array(
                show_file('files created by BOINC Client', $mp0775),
                show_file('files created by project apps', $pp0775)
            ))
        )),
        show_dir(1, 'slots', $mm0775, array(
            show_dir(2, '0', $mp0775, array(
                show_file('files created by BOINC Client', $mp0775),
                show_file('files created by project apps', $pp0775)
            ))
        )),
        show_dir(1, 'switcher (directory)', $mm0550, array(
            show_file('switcher (executable)', $pp6551),
            show_file('setprojectgrp (executable)', $mp2500)
        )),
        show_dir(1, 'locale', $mm0550, array(
            show_dir(2, 'de', $mm0550, array(
                show_file('BOINC Manager.mo', $mm0550),
                show_file('wxstd.mo', $mm0550)
            ))
        )),
        show_file('account_*.xml', $mm0775),
        show_file('acct_mgr_login.xml', $mm0775),
        show_file('client_state.xml', $mm0775),
        show_file('gui_rpc_auth.cfg', $mm0770),
        show_file('sched_reply*', $mm0775),
        show_file('sched_request*', $mm0775)
    ));

echo "<br><br>";

echo
    show_dir(0, 'BOINC executables', $ua0555, array(
        show_file('BOINC Manager', $mm2555),
        show_file('BOINC Client', $mm6555)
    ));
    
echo "

<p>Implementation notes:

<ul>
<li>BOINC Client runs setuid and setgid to <b>boinc_master:boinc_master</b>.  
<li>BOINC Client uses the helper application <i>setprojectgrp</i> to 
set project and slot files and directories to group <b>boinc_project</b>.  
<li>BOINC Client does not directly execute project applications.
It runs the helper application <i>switcher</i>, 
passing the request in the argument list.
<i>switcher</i> runs setuid <b>boinc_project</b> and setgid 
<b>boinc_project</b>,
so all project applications inherit user and group <b>boinc_project</b>.  
This blocks project applications from accessing unauthorized files.
<li>BOINC Manager runs setgid to group <b>boinc_master</b>.
It can access all files in group <b>boinc_master</b>.  
It runs as the user who launched it,
which is necessary for a number of GUI features to work correctly.  
Although this means that BOINC Manager cannot modify files
created by project applications, there is no need for it to do so.  
<li>BOINC Manager and BOINC Client set their umasks to 002,
which is inherited by all child applications.
The default permissions for all files and directories they create prevent
modification outside the user and group.
Because files are world-readable, BOINC Client can read files written by projects.
Third-party add-ons can also read BOINC data files.
<li>Non-admin users cannot directly modify BOINC or project files.
They can modify these files only by running the BOINC Manager and Client.  
<li>The <i>switcher</i> application is inside the <i>switcher</i> directory.
This directory is accessible only by user and group <b>boinc_master</b>,
so that project applications cannot modify the <i>switcher</i> 
application's permissions or code.
<li>Users with admin access are members of groups <b>boinc_master</b>
and <b>boinc_project</b> so that they do have 
direct access to all BOINC and project files
to simplify maintenance and administration.
<li>The RPC password file <i>gui_rpc_auth.cfg</i>
is accessible only by user and group <b>boinc_master</b>.
In other words, only BOINC Manager, BOINC Client and
authorized administrative users can read or modify it,
limiting access to most BOINC RPC functions.
<li>BOINC Manager restricts certain functions to authorized users:
Attach to Project, Detach from Project, Reset Project, Abort Task,
Abort Transfer, Update Account Manager.  
If an unauthorized user requests these functions,
the Manager requires password authentication.
<li>On Macintosh computers, the actual directory structure
of the BOINC Manager application bundle is more complex 
than implied by the box <i>BOINC executables</i> in the BOINC
tree diagram shown above.
<li>Some Macintosh system administrators may wish to limit which users
can perform BOINC Manager functions (Activity Menu, etc.).
This can be done by moving BOINC Manager out of the
<b>/Applications</b> directory into a directory with restricted access.
</ul>
</p>
";

page_tail();
?>
