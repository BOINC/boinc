<?php

require_once("docutil.php");
require_once("versions.inc");

$client_info = $_SERVER['HTTP_USER_AGENT'];

function latest_version($p) {
    foreach ($p['versions'] as $i=>$v) {
        if (!$dev && is_dev($v)) continue;
        return $v;
    }
}

function download_link($pname) {
    global $platforms;
    global $url_base;
    $p = $platforms[$pname];
    $v = latest_version($p);
    $file = $v['file'];
    $long_name = $p['name'];
    $num = $v['num'];
    $path = "dl/$file";
    $url = $url_base.$file;
    $dlink = "<a href=$url>$file</a>";
    $s = number_format(filesize($path)/1000000, 2);

    return "
        <table border=4 cellpadding=10><tr><td bgcolor=ccccff>
        <a href=$url><font size=4><u>Download BOINC</u></font></a>
        <br>
        $num for $long_name ($s MB)
        </td></tr> </table>
    ";
}

function link_row($pname) {
    echo "<tr><td>";
    if ($pname=='win') {
        echo "<img src=images/ico-win.png> <b>Windows</b>";
    } else if ($pname=='mac') {
        echo "<img src=images/ico-osx-uni.png> <b>Mac OS X</b>";
    } else if ($pname=='linux') {
        echo "<img src=images/ico-tux.png> <b>Linux</b>";
    }
    echo "</td><td>";
    echo download_link($pname);
    echo "</td></tr>
    ";
}

function show_download($pname) {
    echo "
        <table cellpadding=30><tr><td>
        <img valign=top hspace=8 align=right src=images/boinc_screen.png>
        BOINC is a program that lets you donate
        your idle computer time to science projects like
        SETI@home, Climateprediction.net, Rosetta@home,
        World Community Grid, and many others.
        <p>
        After installing BOINC on your computer,
        you can connect it to as many of these projects as you like.
        <p>
    ";
    if ($pname) {
        echo download_link($pname);
    } else {
        echo "<table cellpadding=8>
        ";
        link_row('win');
        link_row('mac');
        link_row('linux');
        echo "</table>
        ";
    }
    echo "
        <p>
        <a href=system_requirements.php>System requirements</a>
        | <a href=release_notes.php>Release notes</a>
    ";
    if ($pname) {
        echo " | <a href=download2.php?all_platforms=1>Other systems</a>
        ";
    } else {
        echo " | <a href=download.php>All versions</a>
        ";
    }
    echo " </td></tr></table>
    ";
}

if ($_GET['xml']) {
    Header("Location: download_all.php?xml=1");
    exit();
}

page_head("BOINC: compute for science");

if ($_GET['all_platforms']) {
    show_download(null);
} else if (strstr($client_info, 'Windows')) {
    show_download('win');
} else if (strstr($client_info, 'Mac')) {
    show_download('mac');
} else if (strstr($client_info, 'Linux')) {
    show_download('linux');
} else {
    show_download(null);
}
page_tail();

?>
