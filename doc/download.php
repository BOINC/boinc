<?php

require_once("docutil.php");

$xml = $_GET["xml"];
$dev = $_GET["dev"];

require_once("versions.inc");


function dl_item($x, $y) {
    global $light_blue;
    echo "<tr><td valign=top  align=right width=30% bgcolor=$light_blue>$x</td>
        <td>$y</td></tr>
    ";
}

function show_detail($v) {
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = "http://boinc.berkeley.edu/$path";
    $dlink = "<a href=$url>$file</a>";
    //$md = md5_file($path);
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = $v["type"];
    $features = $v["features"];
    $bugs = $v["bugs"];

    list_start();
    dl_item("File (click to download)", "$dlink ($s MB)");
    dl_item("Version number", $num);
    dl_item("Release date", $date);
    dl_item("Installer type", $type);
    //dl_item("MD5 checksum of download file", $md);
    if ($features) {
        dl_item ("New features", $features);
    }
    if ($bugs) {
        dl_item ("Known problems", $bugs);
    }
    list_end();
}

// show details on a version if URL indicates
//
$pname = $_GET["platform"];
if ($pname) {
    $i = $_GET["i"];
    $p = $platforms[$pname];
    $long_name = $p["name"];
    $va = $p["versions"];
    $v = $va[$i];
    $num = $v["num"];
    page_head("BOINC version $num for $long_name");
    show_detail($v);
    page_tail();
    exit();
}

function show_version_xml($v, $long_name) {
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = "http://boinc.berkeley.edu/$path";
    $dlink = "<a href=$url>$file</a>";
    //$md = md5_file($path);
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = $v["type"];
    $features = $v["features"];
    $bugs = $v["bugs"];
    $bugs = htmlspecialchars($bugs);
    $features = htmlspecialchars($features);
    echo "
<version>
    <platform>$long_name</platform>
    <description>$status</description>
    <date>$date</date>
    <version_num>$num</version_num>
    <url>$url</url>
    <filename>$file</filename>
    <size_mb>$s</size_mb>
    <installer>$type</installer>
    <features>$features</features>
    <issues>$bugs</issues>
</version>
";
//    <md5>$md</md5>
}

function is_dev($v) {
    return (strstr($v["status"], "Development") != null);
}

function show_version($pname, $i, $v) {
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    if (is_dev($v)) {
        $status = $status."
            <font color=dd0000><b>
            (MAY BE UNSTABLE - USE ONLY FOR TESTING)
            </b></font>
        ";
    }
    $path = "dl/$file";
    $s = number_format(filesize($path)/1000000, 2);
    $type = $v["type"];
    echo "<tr><td width=3%><nobr>
        $num</td><td> $status
        </nobr>
        </td>
        <td>
        <a href=dl/$file><b>Download</b></a> ($s MB)
        </td>
        <td>
        $type
        </td>
        <td width=1%>
        <a href=download.php?platform=$pname&i=$i>details</a>
        </td>
        </tr>
    ";
}

function show_platform($short_name, $p, $dev) {
    $long_name = $p["name"];
    list_bar($long_name);
    foreach ($p["versions"] as $i=>$v) {
        if ($dev || !is_dev($v)) {
            show_version($short_name, $i, $v);
        }
    }
}

function show_platform_xml($short_name, $p) {
    $long_name = $p["name"];
    foreach ($p["versions"] as $i=>$v) {
        show_version_xml($v, $long_name);
    }
}

if ($xml) {
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
<versions>
";
    foreach($platforms as $short_name=>$p) {
        show_platform_xml($short_name, $p);
    }
    echo "</versions>\n";
} else {
    page_head("Download BOINC client software");
    echo "<table border=2 cellpadding=4 width=100%>";
    foreach($platforms as $short_name=>$p) {
        show_platform($short_name, $p, $dev);
    }
    list_end();
    echo "
        <p>
        If your computer is not of one of these types, you can
        <ul>
        <li> <a href=anonymous_platform.php>make your own client software</a> or
        <li> <a href=download_other.php>download executables from a third-party site</a>
            (available for Solaris/Opteron, Linux/Opteron, Linux/PPC, HP-UX, and FreeBSD, and others).
        </ul>
        <p>
        Download information is also available in
        <a href=download.php?xml=1>XML format</a>.
        <p>
        Versions 4.27 and earlier may contain an erroneous
        End-User License Agreement.
        The correct text is <a href=eula.txt>here</a>.
    ";
    page_tail();
}

?>
