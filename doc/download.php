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
        echo "<img src=images/ico-tux.png> <b>Linux/x86</b>";
    }
    echo "</td><td>";
    echo download_link($pname);
    echo "</td></tr>
    ";
}

$apps = array(
    array('classic.jpg', 180, 143),
    array('cpdn_200.jpg', 200, 147),
    array('eah_200.png', 200, 150),
);

function show_pictures() {
    global $apps;
    shuffle($apps);
    $a0 = $apps[0];
    $a1 = $apps[1];
    $f0 = $a0[0];
    $f1 = $a1[0];
    echo "
        <div style=\"max-height: 300px\">
        <img src=images/mgrwork.png><br>
        <div style=\"position:relative; top:-80px; left:30px\">
            <img src=images/$f0><br>
        </div>
        <div style=\"position:relative; top:-160px; left:70px\">
            <img src=images/$f1><br>
        </div>
        </div>
    ";
}

function show_download($pname) {
    echo "
        <table cellpadding=10><tr><td valign=top>
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
        <a href=system_requirements.php><nobr>System requirements</nobr></a>
        | <a href=release_notes.php><nobr>Release notes</nobr></a>
    ";
    if ($pname) {
        echo " | <a href=download.php?all_platforms=1><nobr>Other systems</nobr></a>
        ";
    } else {
        echo " | <a href=download_all.php></nobr>All versions</nobr></a>
        <p>
        If your computer is not of one of the above types, you can
        <ul>
        <li> <a href=anonymous_platform.php>make your own client software</a> or
        <li> <a href=download_other.php>download executables from a third-party site</a>
        (available for Solaris/Opteron, Linux/Opteron, Linux/PPC, HP-UX, and FreeBSD, and others).
        </ul>
        ";
    }
    echo "
        <p>
        </td><td valign=top>
    ";
    show_pictures();
    echo "
        </td>
        </tr></table>
        <hr>
        <font size=-2>
        Note: files are downloaded from mirror servers
        at boinc.berkeley.edu, morel.mit.edu, einstein.aei.mpg.de,
        einstein.astro.gla.ac.uk, and einstein.aset.psu.edu
        (thanks to these institutions).
        The server is chosen randomly -
        if a download fails, reload this page and try again.
        </font>
    ";
}

if ($_GET['xml']) {
    $args = strstr($_SERVER['REQUEST_URI'], '?');
    Header("Location: download_all.php$args");
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
