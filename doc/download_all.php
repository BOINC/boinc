<?php

// show information about downloadable BOINC client software
//
// URL options:
// xml=1            Show results as XML (other options are ignored)
// dev=1            Show "development" versions
// min_version=x    show no versions earlier than x
// max_version=x    show no versions later than x
// version=x        show version x
// platform=x       show only versions for platform x (win/mac/linux/solaris)

require_once("docutil.php");

$xml = $_GET["xml"];
$dev = $_GET["dev"];
if (!$xml) $dev=1;
$pname = $_GET["platform"];
$min_version = $_GET["min_version"];
$max_version = $_GET["max_version"];
$version = $_GET["version"];
$type_name = $_GET["type"];

require_once("versions.inc");

if ($dev) {
    $url_base = "dl/";
}

function dl_item($x, $y) {
    global $light_blue;
    echo "<tr><td valign=top  align=right width=30% bgcolor=$light_blue>$x</td>
        <td>$y</td></tr>
    ";
}

function show_detail($v) {
    global $url_base;
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = $url_base.$file;
    $dlink = "<a href=$url>$file</a>";
    //$md = md5_file($path);
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = type_text($v["type"]);
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


function show_version_xml($v, $p) {
    global $url_base;
    $name = $p["name"];
    $dbname = $p["dbname"];
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = $url_base.$file;
    $dlink = "<a href=$url>$file</a>";
    //$md = md5_file($path);
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = type_text($v["type"]);
    $features = $v["features"];
    $bugs = $v["bugs"];
    $bugs = htmlspecialchars($bugs);
    $features = htmlspecialchars($features);
    echo "
<version>
    <platform>$name</platform>
    <dbplatform>$dbname</dbplatform>
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

function show_version($pname, $i, $v) {
    global $url_base;
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
    $type_text = type_text($type);
    echo "<tr><td width=3%><nobr>
        $num</td><td> $status
        </nobr>
        </td>
        <td>
        <a href=".$url_base.$file."><b>Download</b></a> ($s MB)
        </td>
        <td>
        Instructions: $type_text
        </td>
        <td width=1%>
        <a href=download_all.php?platform=$pname&version=$num&type=$type><nobr>version details</nobr></a>
        </td>
        </tr>
    ";
}

function show_platform($short_name, $p, $dev) {
    global $min_version;
    global $max_version;
    $long_name = $p["name"];
    if ($p["url"]) {
        $url = $p["url"];
        $long_name .= " <a href=$url><font size=-2>details</a>";
    }
    list_bar($long_name);
    foreach ($p["versions"] as $i=>$v) {
        if ($min_version && strcmp($v['num'], $min_version)<0) continue;
        if ($max_version && strcmp($v['num'], $max_version)>0) continue;
        if (!$dev && is_dev($v)) continue;
        show_version($short_name, $i, $v);
    }
}

function show_platform_xml($short_name, $p, $dev) {
    foreach ($p["versions"] as $i=>$v) {
        if (!$dev && is_dev($v)) continue;
        show_version_xml($v, $p);
    }
}

// show details on a version if URL indicates
//
if ($pname && $version) {
    $p = $platforms[$pname];
    if (!$p) {
        error_page("platform not found");
    }
    $long_name = $p["name"];
    $va = $p["versions"];
    foreach ($va as $v) {
        if ($v['num'] == $version && $type_name==$v['type']) {
            page_head("BOINC version $version for $long_name");
            show_detail($v);
            page_tail();
            exit();
        }
    }
    error_page( "version not found\n");
}

if ($xml) {
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
<versions>
";
    foreach($platforms as $short_name=>$p) {
        show_platform_xml($short_name, $p, $dev);
    }
    echo "</versions>\n";
} else {
    if ($pname) {
        $p = $platforms[$pname];
        $name = $p['name'];
        page_head("Download BOINC client software for $name");
        echo "<table border=2 cellpadding=4 width=100%>";
        show_platform($pname, $p, $dev);
        list_end();
    } else {
        page_head("Download BOINC client software");
        echo "
            We are now using mirrored download servers at partner institutions.
            Your download will come from a randomly-chosen server.
            Thanks to these partners for their help.
            <b>If you have trouble downloading a file,
            please reload this page in your browser and try again.
            This will link to a different download mirror and may
            fix the problem.</b>
            <p>
            <table border=2 cellpadding=4 width=100%>
        ";
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
            BOINC is not available for Mac OS 9 or earlier.
            There are no plans to develop an OS 9 version.
            <p>
            The Windows BOINC client can be
            <a href=win_deploy.php>deployed across a Windows network
            using Active Directory</a>.
        ";
    }
    echo "
        <p>
        Download information can be restricted by
        platform and/or version number, 
        and can be obtained in XML format.
        <a href=download_info.php>Details</a>.
    ";
    page_tail();
}

?>
