<?php

$basic_tests = array(
    array("New install",
        "Install BOINC client with no existing install"
    ),
    array("Existing install",
        "Install BOINC client with existing install"
    ),
);


$platforms = array(
    array("winxp", "Win XP", 10,),
    array("winxp_x64", "Win XP x64", 5,),
    array("win2000", "Win 2000", 5,),
    array("win98", "Win 98/ME", 5,),
    array("win2003", "Win 2003 server", 5,),
    array("winvista", "Windows Vista", 5,),
    array("winvista_x64", "Windows Vista x64", 5,),
    array("mac103", "Mac OS X 10.3", 5,),
    array("mac104", "Mac OS X 10.4", 5,),
    array("mac105", "Mac OS X 10.5", 5,),
    array("linuxcmdline", "Linux x86 command-line", 5,),
    array("linuxgraphical", "Linux x86 graphical", 5,),
    //array("linuxcmdline_x64", "Linux x64 command-line", 5,),
    array("linuxgraphical_x64", "Linux x64 graphical", 5,),
);

$test_groups = array(
    array("general", "General tests", 10,),
    array("clean", "Clean install", 10,),
    array("transfer_restart", "File transfer restart", 5,),
    array("preference", "Preferences tests", 5,),
    array("v6_win_compat_private", "Windows compatibility/private install tests", 5,),
    array("v6_win_secure_private", "Windows secure/private install tests", 5,),
    array("v6_win_secure_public", "Windows secure/public install tests", 5,),
    array("v6_migraton", "Windows data file migration tests", 5,),
    array("modem", "Tests for modem-connected connections", 5,),
    array("firewall", "Tests for computers with personal firewalls", 5,),
    array("proxyhttp", "Tests for computers connected by HTTP proxy", 5,),
    array("proxysocks", "Tests for computers connected by SOCKS proxy", 2,),
    array("laptop", "Tests for laptops", 5,),
    array("screensaver", "Screensaver tests", 5,),
    array("attach_wizard", "BOINC Manager 'Attach to Project' tests", 5,),
    array("gui_rpc_auth", "GUI RPC authentication", 5,),
    array("acct_mgr", "Account manager functions", 5,),
    array("trickle", "Trickle messages and intermediate file upload", 5,),
);

// newer versions first
$versions = array(
    "6.2.1",
    "6.1.16",
    "6.1.15",
    "6.1.14",
    "6.1.12",
    "6.1.10",
    "6.1.8",
    "6.1.7",
    "6.1.6",
    "5.10.45",
    "5.10.44",
    "5.10.43",
    "5.10.42",
    "5.10.41",
    "5.10.40",
    "5.10.39",
    "5.10.38",
    "5.10.35",
    "5.10.34",
    "5.10.33",
    "5.10.32",
    "5.10.30",
    "5.10.28",
    "5.10.27",
    "5.10.26",
    "5.10.24",
    "5.10.23",
    "5.10.21",
    "5.10.20",
    "5.10.19",
    "5.10.18",
    "5.10.17",
    "5.10.16",
    "5.10.15",
    "5.10.14",
    "5.10.13",
    "5.10.12",
    "5.10.11",
    "5.10.10",
    "5.10.8",
    "5.10.7",
    "5.10.6",
    "5.10.5",
    "5.10.4",
    "5.10.3",
    "5.10.2",
    "5.10.1",
    "5.10.0",
    "5.9.12",
    "5.9.11",
    "5.9.10",
    "5.9.9",
    "5.9.8",
    "5.9.7",
    "5.9.5",
    "5.9.4",
    "5.9.3",
    "5.8.17",
    "5.8.16",
    "5.8.15",
    "5.8.14",
    "5.8.13",
    "5.8.12",
    "5.8.11",
    "5.8.9",
    "5.8.8",
    "5.8.7",
    "5.8.6",
    "5.8.5",
    "5.8.4",
    "5.8.3",
    "5.8.2",
    "5.8.1",
    "5.8.0",
    "5.7.5",
    "5.7.4",
    "5.7.3",
    "5.7.2",
    "5.7.1",
    "5.6.5",
    "5.6.4",
    "5.6.3",
    "5.6.2",
    "5.6.1",
    "5.6.0",
    "5.5.16",
    "5.5.15",
    "5.5.14",
    "5.5.13",
    "5.5.12",
    "5.5.11",
    "5.5.10",
    "5.5.9",
    "5.5.8",
    "5.5.6",
    "5.5.5",
    "5.4.9",
    "5.4.8",
    "5.4.7",
    "5.4.6",
    "5.4.5",
    "5.4.4",
    "5.4.3",
    "5.4.2",
    "5.4.1",
    "5.4.0",
    "5.3.31",
    "5.3.30",
    "5.3.29",
    "5.3.28",
    "5.3.27",
    "5.3.26",
    "5.3.24",
    "5.3.23",
    "5.3.22",
    "5.3.21",
    "5.3.20",
    "5.3.16",
    "5.3.15",
    "5.2.15",
    "5.2.14",
    "5.2.13",
    //"5.2.2",
    //"5.2.1",
    //"5.2.0",
    //"5.1.10",
    //"5.1.9",
    //"5.1.8",
    //"5.1.6",
    //"5.1.5",
    //"5.1.4",
    //"5.1.3",
    //"5.1.2",
    //"5.1.1",
    //"4.72",
);

function current_version() {
    global $versions;
    return $versions[0];
}

function reports_for_version($v) {
    $rs = array();
    $r = mysql_query("select * from test_report where version='$v' and status<>3");
    while ($rep = mysql_fetch_object($r)) {
        $rs[] = $rep;
    }
    return $rs;
}

function test_count($x) {
    $n = 0;
    foreach($x as $y) {
        $n += $y[2];
    }
    return $n;
}

function fraction_left($v) {
    global $platforms;
    global $test_groups;
    $ps = $platforms;
    $gs = $test_groups;
    $total_tests = test_count($ps) + test_count($gs);

    $rs = reports_for_version($v);
    foreach ($rs as $r) {
        for ($i=0; $i<count($ps); $i++) {
            if ($r->platform == $ps[$i][0]) {
                if ($ps[$i][2]>0) {
                    $ps[$i][2]--;
                }
                break;
            }
        }
        for ($i=0; $i<count($gs); $i++) {
            if ($r->test_group == $gs[$i][0]) {
                if ($gs[$i][2]>0) {
                    $gs[$i][2]--;
                }
                break;
            }
        }
    }
    $tests_left = test_count($ps) + test_count($gs);
    return $tests_left/$total_tests;
}

function platform_name($short_name) {
    global $platforms;
    for ($i=0; $i<count($platforms); $i++) {
        $p = $platforms[$i];
        if ($short_name == $p[0]) return $p[1];
    }
    return $short_name;
}

function test_group_name($short_name) {
    global $test_groups;
    for ($i=0; $i<count($test_groups); $i++) {
        $p = $test_groups[$i];
        if ($short_name == $p[0]) return $p[1];
    }
    return $short_name;
}

function show_status_select($t, $user, $v, $p) {
    $tgname = $t[0];
    $query = "select * from test_report where userid=$user->id and version='$v' and platform='$p' and test_group='$tgname'";
    $result = mysql_query($query);
    $tr = mysql_fetch_object($result);
    if ($tr) {
        if ($tr->status==0) $ch0 = "checked";
        if ($tr->status==1) $ch1 = "checked";
        if ($tr->status==2) $ch2 = "checked";
        if ($tr->status==3) $ch3 = "checked";
        $comment = $tr->comment;
    } else {
        $ch3 = "checked";
    }

    echo "
        <td><input type=radio name=".$tgname."_status value=3 $ch3></td>
        <td><input type=radio name=".$tgname."_status value=0 $ch0></td>
        <td><input type=radio name=".$tgname."_status value=1 $ch1></td>
        <td><input type=radio name=".$tgname."_status value=2 $ch2></td>
        <td><textarea rows=4 cols=60 name=".$tgname."_comment>$comment</textarea></td>
    ";
}

function show_test_groups($user, $v, $p) {
    global $test_groups;
    echo "<table cellpadding=4 border=1>
        <tr><th><a href=http://boinc.berkeley.edu/test_matrix.php>Test group</a></th>
        <th>Not<br> tested</th>
        <th>No<br>bugs<br>found</th>
        <th>Minor<br>bugs<br>found</th>
        <th>Major<br>bugs<br>found</th>
        <th>Comments (include BOINC trac ticket number if applicable)</th>
        </tr>
    ";
    for ($i=0; $i<count($test_groups); $i++) {
        $t = $test_groups[$i];
        echo "<tr><td>$t[1]</td>\n";
        show_status_select($t, $user, $v, $p);
        echo "</tr>\n";
    }
    echo "</table>\n";
}

function show_version_select() {
    global $versions;
    echo "<select name=version>\n";
    for ($i=0; $i<count($versions); $i++) {
        $p = $versions[$i];
        echo "<option value=$p>$p\n";
    }
    echo "</select>\n";
}

function show_platform_select() {
    global $platforms;
    for ($i=0; $i<count($platforms); $i++) {
        $p = $platforms[$i];
        echo "<br><input type=radio name=platform value=$p[0]>$p[1]\n";
    }
}


function show_platform_summary() {
    global $versions;
    global $platforms;
    for ($i=0; $i<count($platforms); $i++) {
        $p = $platforms[$i];
        echo $p[1]."-";
        for ($n=0; $n<count($versions); $n++) {
            $v = $versions[$n];
            echo "<a href=\"test_details.php?platform=".$p[0].
                 "&version=".$v."\">".$v."</a> ";
            }
        echo "<br>\n";
    }
}

function show_platform_details($platform) {
     echo $platform->platform, $platform->version, $platform->status;
}

?>
