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
$pp2 = prot('boinc_master', 'boinc_master', '0700');
$pp3 = prot('boinc_project', 'boinc_project', '0770+setuid');

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
    show_dir('BOINC data', $pp, array(
        show_dir('projects', $pp, array(
            show_dir('setiathome.berkeley.edu', $pp, array(
                show_file('*', $pp)
            ))
        )),
        show_dir('slots', $pp, array(
            show_dir('0', $pp, array(
                show_file('*', $pp)
            ))
        )),
        show_dir('switcher', $pp2, array(
            show_file('switcher', $pp3)
        )),
        show_file('account_*.xml', $pp),
        show_file('acct_mgr_login.xml', $pp),
        show_file('client_state.xml', $pp),
        show_file('gui_rpc_auth.cfg', $pp),
        show_file('sched_reply*', $pp),
        show_file('sched_request*', $pp),
    ));

echo
    show_dir('BOINC executables', $pp, array(
        show_file('BOINC Manager', $pp),
        show_file('Core client', $pp),
    ));
?>
