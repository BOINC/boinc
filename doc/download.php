<?php

// NOTE: the core client fetches download.php?xml=1 every so often;
// don't break this!!!

require_once("docutil.php");
require_once("versions.inc");
require_once("../html/inc/translation.inc");

$client_info = $_SERVER['HTTP_USER_AGENT'];

function latest_version($p) {
    foreach ($p['versions'] as $i=>$v) {
        if (!$dev && is_dev($v)) continue;
        return $v;
    }
    foreach ($p['versions'] as $i=>$v) {
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
    $dlink = "<a href=\"$url\">$file</a>";
    $s = number_format(filesize($path)/1000000, 2);

    echo "
        <table border=4 cellpadding=10><tr><td class=heading>
        <a href=\"$url\"><font size=4><u>".tra("Download BOINC")."</u></font></a>
        <br>".
        sprintf(tra("%s for %s (%s MB)"), $num, $long_name, $s )."
        </td></tr> </table>
    ";
    if ($pname == 'linux'||$pname == 'linuxx64') {
        show_linux_info();
    }
}

function link_row($pname) {
    echo "<tr><td>";
    if ($pname=='win') {
        echo "<img src=\"images/ico-win.png\" alt=\"Windows icon\"> <b>Windows</b>";
    } else if ($pname=='mac') {
        echo "<img src=\"images/ico-osx-uni.png\" alt=\"Mac icon\"> <b>Mac OS X</b>";
    } else if ($pname=='linux') {
        echo "<img src=\"images/ico-tux.png\" alt=\"Linux icon\"> <b>Linux/x86</b>";
    }
    echo "</td><td>";
    download_link($pname);
    echo "</td></tr>
    ";
}

$apps = array(
    array('classic.jpg', 180, 143),
    array('cpdn_200.jpg', 200, 147),
    array('eah_200.png', 200, 150),
    array('rosetta_at_home.jpg', 200, 150),
    array('qah.200x150.png', 200, 150),
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
        <img src=\"images/mgrwork.png\" alt=\"BOINC manager\"><br>
        <div style=\"position:relative; top:-80px; left:30px\">
            <img src=\"images/$f0\" alt=\"BOINC project\"><br>
        </div>
        <div style=\"position:relative; top:-160px; left:70px\">
            <img src=\"images/$f1\" alt=\"BOINC project\"><br>
        </div>
        </div>
    ";
}

function show_download($pname) {
    echo "
        <table cellpadding=10><tr><td valign=top>
        ".tra("BOINC is a program that lets you donate your idle computer time to science projects like SETI@home, Climateprediction.net, Rosetta@home, World Community Grid, and many others. <p> After installing BOINC on your computer, you can connect it to as many of these projects as you like.").
        "<p>"
    ;
    if ($pname) {
        download_link($pname);
    } else {
        echo "<table cellpadding=8>
        ";
        link_row('win');
        link_row('winx64');
        link_row('mac');
        link_row('linux');
        link_row('linuxx64');
        link_row('linuxcompat');
        echo "</table>
        ";
    }
    echo "
        <p>
        <a href=\"wiki/System_requirements\"><span class=nobr>".tra("System requirements")."</span></a>
        | <a href=\"wiki/Release_Notes\"><span class=nobr>".tra("Release notes")."</span></a>
        | <a href=\"wiki/BOINC_Help\"><span class=nobr>".tra("Help")."</span></a>
    ";
    if ($pname) {
        //echo " | <a href=\"download.php?all_platforms=1\"><span class=nobr>".tra("Other systems")."</span></a>
        echo " | <a href=download_all.php><span class=nobr>".tra("All versions")."</span></a>
        ";
    } else {
        echo " | <a href=download_all.php><span class=nobr>".tra("All versions")."</span></a>
        <p>"
        .tra("If your computer is not of one of the above types, you can")."
        <ul>
        <li> ".sprintf(tra("%s make your own client software %s or"), "<a href=anonymous_platform.php>", "</a>")."
        <li> ".sprintf(tra("%s download executables from a third-party site %s (available for Solaris/Opteron, Linux/Opteron, Linux/PPC, HP-UX, and FreeBSD, and others)."), "<a href=download_other.php>", "</a>")."
        </ul>
        ";
    }
    echo "
        | <a href=\"trac/wiki/VersionHistory\">Version history</a>
        | <a href=\"http://boincfaq.mundayweb.com/index.php?view=376\">FAQ</a>
        <p>
        </td><td valign=top>
    ";
    show_pictures();
    echo "
        </td>
        </tr></table>
    ";
}

if ($_GET['xml']) {
    $args = strstr($_SERVER['REQUEST_URI'], '?');
    Header("Location: download_all.php$args");
    exit();
}

page_head(tra("BOINC: compute for science"));

if ($_GET['all_platforms']) {
    show_download(null);
} else if (strstr($client_info, 'Windows')) {
    if (strstr($client_info, 'Win64')||strstr($client_info, 'WOW64')) {
        show_download('winx64');
    } else {
        show_download('win');
    }
} else if (strstr($client_info, 'Mac')) {
    show_download('mac');
} else if (strstr($client_info, 'Linux')) {
    if (strstr($client_info, 'x86_64')) {
        show_download('linuxx64');
    } else {
        show_download('linux');
    }
} else {
    show_download(null);
}

page_tail(true);

?>
