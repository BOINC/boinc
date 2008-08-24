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

function dl_item($x, $y) {
    global $light_blue;
    echo "<tr><td valign=top  align=right width=30% bgcolor=$light_blue>$x</td>
        <td>$y</td></tr>
    ";
}

function version_url($v) {
    global $url_base;
    $file = $v["file"];
    if (is_dev($v)) {
        return "http://boinc.berkeley.edu/dl/$file";
    } else {
        return $url_base.$file;
    }
}

function show_detail($v) {
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = version_url($v);
    $dlink = "<a href=$url>$file</a>";
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = type_text($v["type"]);

    list_start();
    dl_item("File (click to download)", "$dlink ($s MB)");
    dl_item("Version number", $num);
    dl_item("Release date", $date);
    list_end();
}


function show_version_xml($v, $p) {
    $name = $p["name"];
    $dbname = $p["dbname"];
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    $path = "dl/$file";
    $url = version_url($v);
    $dlink = "<a href=$url>$file</a>";
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = type_text($v["type"]);
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
</version>
";
}

function show_version($pname, $i, $v) {
    $num = $v["num"];
    $file = $v["file"];
    $status = $v["status"];
    if (is_dev($v)) {
        $status = $status."
            <br><font color=ff0000><b>
            (MAY BE UNSTABLE - USE ONLY FOR TESTING)
            </b></font>
        ";
    }
    $path = "dl/$file";
    $s = number_format(filesize($path)/1000000, 2);
    $date = $v["date"];
    $type = $v["type"];
    $type_text = type_text($type);
    $url = version_url($v);
    echo "<tr><td width=3%><nobr>
        $num</td><td> $status
        </nobr>
        </td>
        <td>
        <a href=$url><b>Download</b></a> ($s MB)
        </td>
        <td > $date </td>
        </tr>
    ";
}

function show_platform($short_name, $p, $dev) {
    global $min_version;
    global $max_version;
    $long_name = $p["name"];
    $description = $p["description"];
    if ($p["url"]) {
        $url = $p["url"];
        $long_name .= " <a href=$url><font size=-2>details</a>";
    }
    list_bar($long_name, $description);
    foreach ($p["versions"] as $i=>$v) {
        if ($min_version && version_compare($v['num'], $min_version, "<")) continue;
        if ($max_version && version_compare($v['num'], $max_version, ">")) continue;
        if (!$dev && is_dev($v)) continue;
        show_version($short_name, $i, $v);
    }
}

function show_platform_xml($short_name, $p, $dev) {
    foreach ($p["versions"] as $i=>$v) {
        if (!$dev && is_dev($v)) continue;
        // show only those builds that have been around for over
        // two weeks.  Gives us time to address any showstoppers
        // found by the early adopters
        //if (!$dev && ((time() - strtodate($v["date"])) >= 86400*14)) continue;
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
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n
<versions>\n
";
    foreach($platforms as $short_name=>$p) {
        show_platform_xml($short_name, $p, $dev);
    }
    echo "
</versions>\n
";
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
            <table border=2 cellpadding=4 width=100%>
        ";
        foreach($platforms as $short_name=>$p) {
            show_platform($short_name, $p, $dev);
        }
        list_end();
        echo "
            <h3>Other platforms</h3>
            If your computer is not of one of these types, you can
            <ul>
            <li> <a href=trac/wiki/AnonymousPlatform>make your own client software</a> or
            <li> <a href=trac/wiki/DownloadOther>download executables from a third-party site</a>
                (available for Solaris/Opteron, Linux/Opteron, Linux/PPC, HP-UX, and FreeBSD, and others).
            </ul>

            <h3>Linux info</h3>
        ";
        show_linux_info();
    }
    echo "
        <h3>Customizing this page</h3>
        The information on this page can be
        <a href=trac/wiki/DownloadInfo>
        restricted by platform and/or version number,
        or presented  in XML format</a>.
    ";
    page_tail();
}

//?>
