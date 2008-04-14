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
    $dlink = "<a href=$url>$file</a>";
    $s = number_format(filesize($path)/1000000, 2);

    echo "
        <table border=4 cellpadding=10><tr><td class=fieldname>
        <a href=$url><font size=4><u>".tr(DL_DOWNLOAD)."</u></font></a>
        <br>".
        sprintf(tr(DL_VERSION_LNAME_SIZE), $num, $long_name, $s )."
        </td></tr> </table>
    ";
    if ($pname == 'linux'||$pname == 'linuxx64') {
        show_linux_info();
    }
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
        ".tr(DL_WHATS_BOINC)
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
        <a href=trac/wiki/SystemRequirements><nobr>".tr(DL_SYSTEMREQ)."</nobr></a>
        | <a href=trac/wiki/ReleaseNotes><nobr>".tr(DL_RELNOTES)."</nobr></a>
        | <a href=trac/wiki/TroubleshootClient><nobr>".tr(DL_TROUBLE)."</nobr></a>
    ";
    if ($pname) {
        //echo " | <a href=download.php?all_platforms=1><nobr>".tr(DL_OTHERSYS)."</nobr></a>
        echo " | <a href=download_all.php><nobr>".tr(DL_ALLVERSIONS)."</nobr></a>
        ";
    } else {
        echo " | <a href=download_all.php><nobr>".tr(DL_ALLVERSIONS)."</nobr></a>
        <p>"
        .tr(DL_IF_OTHERTYPES)."
        <ul>
        <li> ".sprintf(tr(DL_MAKEYOUROWN),"<a href=anonymous_platform.php>","</a>")."
        <li> ".sprintf(tr(DL_DL_FROM3RDP),"<a href=download_other.php>","</a>")."
        </ul>
        ";
    }
    echo "
        | <a href=trac/wiki/VersionHistory>Version history</a>
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

page_head(tr(DL_DOWNLOAD_TITLE));

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
