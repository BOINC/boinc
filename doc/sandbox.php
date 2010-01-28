<?php
require_once("docutil.php");
page_head("Sandbox design");

echo "
This document describes the permissions structure for 
BOINC on the Macintosh.  It has been updated for BOINC versions 6.8.20 and 6.10.30 and later.
The purpose of this scheme is to 'sandbox' BOINC applications,
i.e. to limit the amount of damage that a malicious
or malfunctioning application can cause.
<p>
In our design, BOINC applications run under a specially-created account
having a minimal set of privileges.
In early versions of BOINC, the applications typically ran as the user who installed BOINC,
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

$pp06610771 = prot('boinc_project', 'boinc_project', '0661 or 0771');
$pp06640775 = prot('boinc_project', 'boinc_project', '0664 or 0775');
$mp2500 = prot('boinc_master', 'boinc_project', '0500+setgid');
$rm4050 = prot('root', 'boinc_master', '0050+setuid');
$rm4555 = prot('root', 'boinc_master', '0555+setuid');
$mm0550 = prot('boinc_master', 'boinc_master', '0550');
$mm0555 = prot('boinc_master', 'boinc_master', '0555');
$mm0444 = prot('boinc_master', 'boinc_master', '0444');
$mm0660 = prot('boinc_master', 'boinc_master', '0660');
$mm0664 = prot('boinc_master', 'boinc_master', '0664');
$mm0771 = prot('boinc_master', 'boinc_master', '0771');
$mp0770 = prot('boinc_master', 'boinc_project', '0770');
$mp0775 = prot('boinc_master', 'boinc_project', '0775');
$mp06610771 = prot('boinc_master', 'boinc_project', '0661 or 0771');
$mp06640775 = prot('boinc_master', 'boinc_project', '0664 or 0775');
$mm2555 = prot('boinc_master', 'boinc_master', '0555');
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
<li>Supplementary groups: none
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
    show_dir(0, 'BOINC data', $mm0771, array(
        show_dir(1, 'projects', $mp0770, array(
            show_dir(2, 'setiathome.berkeley.edu', $mp0775, array(
                show_file('files created by BOINC Client', $mp06610771),
                show_file('files created by project apps', $pp06610771)
            ))
        )),
        show_dir(1, 'slots', $mp0770, array(
            show_dir(2, '0', $mp0775, array(
                show_file('files created by BOINC Client', $mp06610771),
                show_file('files created by project apps', $pp06610771)
            ))
        )),
        show_dir(1, 'switcher (directory)', $mm0550, array(
            show_file('switcher (executable)', $rm4050),
            show_file('setprojectgrp (executable)', $mp2500)
        )),
        show_dir(1, 'locale', $mm0555, array(
            show_dir(2, 'de', $mm0555, array(
                show_file('BOINC Manager.mo', $mm0444),
                show_file('wxstd.mo', $mm0444)
            ))
        )),
        show_file('account_*.xml', $mm0660),
        show_file('acct_mgr_login.xml', $mm0660),
        show_file('client_state.xml', $mm0660),
        show_file('gui_rpc_auth.cfg', $mm0660),
        show_file('sched_reply*', $mm0660),
        show_file('sched_request*', $mm0660),
        show_file('ss_config.xml', $mm0664)
    ));

echo "<br><br>";

echo
    show_dir(0, 'BOINC executables', $ua0555, array(
        show_file('BOINC Manager', $mm2555),
        show_file('BOINC Client', $mm6555),
        show_dir(1, 'screensaver (directory)', $ua0555, array(
            show_file('gfx_switcher (executable)', $rm4555)
        )),
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
<i>switcher</i> runs setuid <b>root</b> and immediately changes its real and 
effective user ID and group ID to <b>boinc_project</b>,
so all project applications inherit user and group <b>boinc_project</b>.  
This blocks project applications from accessing unauthorized files.
<li>In most cases, it is best to avoid running setuid <b>root</b> because 
it can present a security risk.  In this case, however, this is necessary to 
<i>reduce</i> the risk because only the superuser can change the <i>real</i> 
user and group of a process.  This prevents a malicious or malfunctioning 
application from reverting to the user and group who launched BOINC, since any 
process can change its user and group back to the <i>real</i> user and 
group IDs.
<li>BOINC's use of setuid <b>root</b> for the <i>switcher</i> application is 
safe because:
<ul>
<li>The <i>switcher</i> application is inside the <i>switcher</i> directory.
This directory is accessible only by user and group <b>boinc_master</b>,
so that project applications cannot modify the <i>switcher</i> 
application's permissions or code.  This also prevents unauthorized users 
from using <i>switcher</i> to damage or manipulate project files.
<li>The <i>switcher</i> application is readable and executable only by 
group <b>boinc_master</b>; all other access is forbidden.
<li>When it is run, the <i>switcher</i> application immediately changes 
its real and effective user ID and group ID to <b>boinc_project</b>, disabling 
its superuser privileges.
</ul>
<li>As of BOINC Version 6.10.5, BOINC Manager no longer runs setgid to group 
<b>boinc_master</b>, because Mac OS 10.6 does not allow it.  So it can be run 
only by users who are members of group <b>boinc_master</b>. By default, the 
BOINC installer automatically adds all users who are members of group 
<b>admin</b> to group <b>boinc_master</b>, and optionally adds non-admin 
users to group <b>boinc_master</b>.  The Manager runs as the user who 
launched it, which is necessary for a number of GUI features to work correctly.  
Although this means that BOINC Manager cannot modify files created by project 
applications, there is no need for it to do so.  
<li>Starting with BOINC version 6.0, project science applications use a 
separate companion application to display graphics.  These graphics 
applications are launched by the BOINC Manager when the user clicks on 
the <i>Show Graphics</i> button.  Running the graphics application 
with the BOINC Manager's user and group would be a security risk, so 
BOINC Manager uses the <i>switcher</i> application to launch them as 
user and group <b>boinc_project</b>. 
<li>The screensaver also can run the graphics applications.  The Macintosh 
screensaver is launched by the operating system, so it runs as the 
currently logged in user and group.  Since running the science projects' graphics applications 
with this user and group would be a security risk, the screensaver has 
its own embedded helper application <i>gfx_switcher</i> which it uses to 
launch and kill the graphics applications.  
Like the <i>switcher</i> application, <i>gfx_switcher</i> runs setuid 
<b>root</b> and immediately changes its real and effective user ID and 
group ID to <b>boinc_project</b>.
<li>Starting with BOINC version 6.7, a default screenaver graphics application 
is provided with BOINC.  The screensaver (now more properly called the 
<b>screensaver coordinator</b>) runs the default graphics alternating with science 
graphics applications according to a schedule set by the data file ss_config.xml.  
The default graphics are run also when no science graphics are available, such as 
when BOINC is suspended.  The default graphics executable is run as user and group 
<b>boinc_project</b>.  
<li>The BOINC screensaver's use of setuid <b>root</b> for the 
<i>gfx_switcher</i> application is safe because:
<ul>
<li>When it is run, the <i>gfx_switcher</i> application immediately changes 
its real and effective user ID and group ID to <b>boinc_project</b>, disabling 
its superuser privileges.
<li>The <i>gfx_switcher</i> application has very limited functionality.  It 
accepts only three commands as its first argument: 
<ul>
<li><i>launch_gfx</i>: the second argument is the slot number.  It looks for 
a soft-link named <b>graphics_app</b> in the specified slot directory and launches 
the referenced graphics application as user and group <b>boinc_project</b>.
<li><i>default_gfx</i>: launches the default graphics application <i>boincscr</i> 
in the BOINC data directory as user and group <b>boinc_project</b>.
<li><i>kill_gfx</i>: the second argument is the process ID.  It kills the 
application with the process ID; since it is running as user and group 
<b>boinc_project</b>, it can affect only processes belonging to that user.  
This is used to exit all screensaver graphics applications.</ul>
</ul>
<li>To hide account keys from unauthorized users, BOINC Client sets its umask 
to 006 and (as of versions 6.8.20 and 6.10.30) makes all *.xml files at the top level 
directory not world-readable (except ss_config.xml, which must be read by the 
screensaver coordinator).  This means that third-party add-ons cannot read BOINC 
data files; they must use GUI RPCs to access BOINC Data.  
<li>BOINC sets the umask for project applications to 002; the default permissions 
for all files and directories they create prevent modification outside the 
<b>boinc_project</b> user and group.  
<li>Files written by projects are world-readable so that the BOINC Client can read 
them.  But, starting with BOINC versions 6.8.20 and 6.10.30, the slots directory and the projects 
directory are executable (traversable) only by user boinc_master and group 
boinc_projects, to prevent unauthorized users from reading account keys from the 
init_data.xml files. 
<li>Unauthorized users cannot modify BOINC or project files.
<li>Users with admin access are members of groups <b>boinc_master</b>
and <b>boinc_project</b> so that they do have 
direct access to all BOINC and project files
to simplify maintenance and administration.
<li>The RPC password file <i>gui_rpc_auth.cfg</i>
is accessible only by user and group <b>boinc_master</b>.
In other words, only BOINC Manager, BOINC Client and authorized 
users can read or modify it, restricting access to those BOINC RPC functions 
which modify BOINC's operation.  
<li>On Macintosh computers, the actual directory structures
of the BOINC Manager application bundle and the screensaver bundle are 
more complex than implied by the box <i>BOINC executables</i> in the 
BOINC tree diagram shown above.
<li>Some Macintosh system administrators may wish to further limit which users
can perform BOINC Manager functions (Activity Menu, etc.).
This can be done by moving BOINC Manager out of the
<b>/Applications</b> directory into a directory with restricted access.
</ul>
</p>
";

page_tail();
?>
