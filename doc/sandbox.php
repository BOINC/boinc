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

$pp = prot('boinc_project', 'boinc_project', '0770');
$mp = prot('boinc_master', 'boinc_project', '0770');
$mm = prot('boinc_master', 'boinc_master', '0770');
$mm2 = prot('boinc_master', 'boinc_master', '0700');
$mm3 = prot('boinc_master', 'boinc_master', '0555+setuid+setgid');
$mm4 = prot('boinc_master', 'boinc_master', '0775+setgid');
$pp3 = prot('boinc_project', 'boinc_project', '0770+setuid+setgid');

function show_dir($name, $prot, $contents) {
    $x = "
        <table cellpadding=6 border=2>
        <tr>
            <td valign=top>$name <font size=-2>$prot</font></td><td valign=top>
    ";
    foreach ($contents as $c) {
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

echo 
    show_dir('BOINC data', $mm, array(
        show_dir('projects', $mp, array(
            show_dir('setiathome.berkeley.edu', $mp, array(
                show_file('CC-created files', $mp),
                show_file('app-created files', $pp)
            ))
        )),
        show_dir('slots', $mp, array(
            show_dir('0', $mp, array(
                show_file('CC-created files', $mp),
                show_file('app-created files', $pp)
            ))
        )),
        show_dir('locale', $mm, array(
            show_dir('en_US', $mm, array(
                show_file('BOINC Manager.mo', $mm),
                show_file('wxstd.mo', $mm)
            ))
        )),
        show_dir('switcher', $mm2, array(
            show_file('switcher', $pp3)
        )),
        show_file('account_*.xml', $mm),
        show_file('acct_mgr_login.xml', $mm),
        show_file('client_state.xml', $mm),
        show_file('gui_rpc_auth.cfg', $mm),
        show_file('sched_reply*', $mm),
        show_file('sched_request*', $mm),
    ));

echo "<br><br>";

echo
    show_dir('BOINC executables', $mm, array(
        show_file('BOINC Manager', $mm4),
        show_file('Core client', $mm3),
    ));
?>
