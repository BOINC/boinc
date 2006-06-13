<?php

function prot($user, $group, $perm) {
    return "
    <br>
    &nbsp;&nbsp; user: $user
    <br>
    &nbsp;&nbsp; group: $group
    <br>
    &nbsp;&nbsp; protection: $perm";
}

$pp0770 = prot('boinc_project', 'boinc_project', '0770');
$pp6550 = prot('boinc_project', 'boinc_project', '0550+setuid+setgid');
$mm0770 = prot('boinc_master', 'boinc_master', '0770');
$mp0770 = prot('boinc_master', 'boinc_project', '0770');
$mp0750 = prot('boinc_master', 'boinc_project', '0750');
$mm2555 = prot('boinc_master', 'boinc_master', '0555+setgid');
$mm6555 = prot('boinc_master', 'boinc_master', '0555+setuid+setgid');
$mm6770 = prot('boinc_master', 'boinc_master', '0770+setuid+setgid');
$mm0775 = prot('(installing user)', 'admin', '0775');

function show_dir($name, $prot, $contents) {
    $x = "
        <table cellpadding=6 cellspacing=0 border=1 width=100%>
        <tr>
            <td valign=top><b>$name</b> <font size=-2>$prot</font></td><td valign=top>
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
            $name <font size=-2>$prot</font><br>
    ";
}

echo "

<p>The BOINC installer creates two users and two groups:
<ul>
<li>Group: <b>boinc_master</b>
<li>Group: <b>boinc_project</b>
<li>User: <b>boinc_master</b>
<ul>
<li>Primary Group: <b>boinc_master</b>
<li>Supplementary Groups: <b>boinc_project</b>
</ul>
<li>User: <b>boinc_project</b>
<ul>
<li>Primary Group: <b>boinc_project</b>
<li>Supplementary Groups: none
</ul>
</ul>
Both groups <b>boinc_project</b> and <b>boinc_master</b> are added to the Supplementary Groups Lists of those other users who are members of group <b>admin</b>.  This gives admin users full access to all BOINC and project files.
</p>
<p>The following diagram shows user, group and permissions for the BOINC file and directory tree:</p>
";

echo
    show_dir('BOINC data', $mp0750, array(
        show_dir('projects', $mp0750, array(
            show_dir('setiathome.berkeley.edu', $mp0770, array(
                show_file('files created by BOINC Client', $mp0770),
                show_file('files created by project apps', $pp0770)
            ))
        )),
        show_dir('slots', $mp0750, array(
            show_dir('0', $mp0770, array(
                show_file('files created by BOINC Client', $mp0770),
                show_file('files created by project apps', $pp0770)
            ))
        )),
        show_dir('switcher (directory)', $mm0770, array(
            show_file('switcher (executable)', $pp6550)
        )),
        show_dir('locale', $mm0770, array(
            show_dir('de', $mm0770, array(
                show_file('BOINC Manager.mo', $mm0770),
                show_file('wxstd.mo', $mm0770)
            ))
        )),
        show_file('account_*.xml', $mm0770),
        show_file('acct_mgr_login.xml', $mm0770),
        show_file('client_state.xml', $mm0770),
        show_file('gui_rpc_auth.cfg', $mm0770),
        show_file('sched_reply*', $mm0770),
        show_file('sched_request*', $mm0770),
    ));

echo "<br><br>";

echo
    show_dir('BOINC executables', $mm0775, array(
        show_file('BOINC Manager', $mm2555),
        show_file('BOINC Client', $mm6555),
    ));
    
echo "

<p>Implementation notes:

<ul>
<li>BOINC Client runs setuid and setgid to <b>boinc_master:boinc_master</b>.  
<li>Because user <b>boinc_master</b> is a member of both groups <b>boinc_master</b> and <b>boinc_project</b>, 
BOINC Client can set files and directories it creates to either group and has full access to files and 
directories in both groups.
<li>BOINC Client does not directly execute project applications.  It runs the helper application <i>switcher</i>, 
passing the request in the argument list.  <i>switcher</i> runs setuid <b>boinc_project</b> and setgid 
<b>boinc_project</b>, so all project applications inherit user and group <b>boinc_project</b>.  
This blocks project applications from accessing unauthorized files.
<li>BOINC Manager runs setgid to group <b>boinc_master</b>.  It can access all files in group <b>boinc_master</b>.  
It runs as the user who launched it, which is necessary for a number of GUI features to work correctly.  
Although this means that BOINC Manager cannot directly access files created by project applications, there is no 
need for it to do so.  
<li>BOINC Manager and BOINC Client set their umasks to 007, which is inherited by all child applications.  The 
default permissions for all files and directories they create prevent access outside the user and group.
<li>Non-admin users have no direct access to BOINC or project files.  They can access these files only by running 
the BOINC Manager and Client.  
<li>The <i>switcher</i> application is inside the <i>switcher</i> directory.  This directory is accessible only 
by user and group <b>boinc_master</b>, so that project applications cannot modify the <i>switcher</i> 
application's permissions or code.
<li>Users with admin access are members of groups <b>boinc_master</b> and <b>boinc_project</b> so that they do have 
direct access to all BOINC and project files to simplify maintenance and administration.
<li>The RPC password file <i>gui_rpc_auth.cfg</i> is accessible only by user and group <b>boinc_master</b>.  In other 
words, only BOINC Manager, BOINC Client and authorized administrative users can read or modify it, limiting access to 
most BOINC RPC functions.
<li>BOINC Manager restricts certain functions to authorized users: Attach to Project, Detach from Project, Reset Project.  
If an unauthorized user requests these functions, the Manager requires password authentication.
<li>On Macintosh computers, the actual directory structure of the BOINC Manager application bundle is more complex 
than implied by the box <i>BOINC executables</i> in the BOINC tree diagram shown above.
<li>Some Macintosh system administrators may wish to limit which users can perform BOINC Manager functions (Activity Menu, 
etc.).  This can be done by moving BOINC Manager out of the <b>/Applications</b> directory into a directory with restricted access.
</ul>
</p>
";

?>
