<?php

require_once("docutil.php");

$xml = $_GET["xml"];

if ($xml) {
} else {
    page_head("Download BOINC client software");
}

function xecho($x) {
    global $xml;
    if (!$xml) {
        echo $x;
    }
}

function dl_item($x, $y) {
    echo "<tr><td valign=top  align=right width=30% bgcolor=d8d8ff>$x</td>
        <td>$y</td></tr>
    ";
}

function version($number, $desc, $filename, $date, $installer, $issues=null) {
    global $xml;
    $path = "dl/$filename";
    $url = "http://boinc.berkeley.edu/$path";
    $dlink = "<a href=$url>$filename</a>";
    $md = md5_file($path);
    $s = number_format(filesize($path)/1000000, 2);

    if ($xml) {
        echo "
<version>
    <description>$desc</description>
    <date>$date</date>
    <version_num>$number</version_num>
    <url>$url</url>
    <filename>$filename</filename>
    <size_mb>$s</size_mb>
    <md5>$md</md5>
    <installer>$installer</installer>
    <issues>$issues</issues>
</version>
";
    } else {
        list_start();
        list_bar($desc);
        dl_item("File (click to download)", "$dlink ($s MB)");
        dl_item("Version number", $number);
        dl_item("Release date", $date);
        dl_item("Installer type", $installer);
        dl_item("MD5 checksum of download file", $md);
        if ($issues) {
            dl_item ("Known issues", $issues);
        }
        list_end();
    }
}

function win_old() {
    return "<a href=client_windows.php>Single-mode Windows installer</a>";
}

function win_new() {
    return "<a href=win_install.php>Multi-mode Windows installer</a>";
}

function bare_core() {
    return "<a href=bare_core.php>Core client only (command-line)</a>";
}

function sea() {
    return "<a href=sea.php>Self-extracting archive</a>";
}

function mac_simple() {
    return "<a href=menubar.php>Menubar (simple GUI)</a>";
}

function mac_advanced() {
    return "<a href=mac_advanced.php>Advanced GUI</a>";
}

xecho( "
BOINC client software is available for
<a href=#windows>Windows</a>,
<a href=#mac>Mac OS X</a>,
<a href=#linux>Linux/x86</a> and
<a href=#solaris>SPARC/Solaris</a>.
Click on your computer type, or scroll down.
<p>
    If your computer is not of one of these types, you can
    <ul>
    <li> <a href=anonymous_platform.php>download the BOINC source code and compile it yourself</a> or
    <li> <a href=download_other.php>download executables from a third-party site</a>.
    </ul>
    <p>

<a name=windows>
<h2>Microsoft Windows (all versions, Windows 95 and later)</h2>
)";
version(
    "4.25",
    "Recommended version",
    "boinc_4.25_windows_intelx86.exe",
    "3 Mar 2005",
    win_new(),
    "<ul>
    <li> The BOINC screensaver conflicts with Microsoft Intellitype software.
    <li>
    Applications that were built before October 2004 do not
    display screensaver graphics
    with the Service or Shared install type,
    or the Single-user install type with the password protect screensaver
    option on NT based machines.
    <li>
    If BOINC runs at the same time as Windows XP 3D screensavers,
    the system becomes sluggish and unresponsive.
    </ul>
    If you experience any of these problems,
    we recommend using BOINC version 4.19.
    "
);
version(
    "4.19",
    "Older version",
    "boinc_4.19_windows_intelx86.exe",
    "25 Jan 2005",
    win_old(),
    "<ul>
    <li> Doesn't work with some HTTP proxies (fixed in later versions).
    </ul>
    "
);
xecho( "
<a name=mac>
<h2>Macintosh OS/X (10.2 and later)</h2>
)";
version(
    "4.19",
    "Recommended version",
    "boinc_4.19_powerpc-apple-darwin.gz",
    "25 Jan 2005",
    bare_core()
);
version(
    "4.25",
    "Development version (simple GUI)",
    "BOINC_Menubar_4.25_mac.zip",
    "10 Mar 2005",
    mac_simple()
);
version(
    "4.25",
    "Development version (command line)",
    "boinc_4.25_powerpc-apple-darwin.gz",
    "3 Mar 2005",
    bare_core()
);

xecho("
<a name=linux>
<h2>Linux/x86</h2>
)";
version(
    "4.19",
    "Recommended version",
    "boinc_4.19_i686-pc-linux-gnu.gz",
    "25 Jan 2005",
    bare_core()
);
version(
    "4.27",
    "Development version",
    "boinc_4.27_i686-pc-linux-gnu.sh",
    "16 Mar 2005",
    sea()
);
xecho( "
<a name=solaris>
<h2>Solaris/SPARC</h2>
)";
version(
    "4.19",
    "Recommended version",
    "boinc_4.19_sparc-sun-solaris2.7.gz",
    "25 Jan 2005",
    bare_core()
);
version(
    "4.26",
    "Development version",
    "boinc_4.26_sparc-sun-solaris2.7.sh",
    "10 Mar 2005",
    sea()
);

xecho("
    <h2>End-User License Agreement</h2>
    Versions 4.27 and earlier may contain an erroneous
    End-User Licence Agreement.
    The correct text is:
    <pre>
License Agreement

Please carefully read the following terms and conditions
before using this software.  Your use of this software indicates
your acceptance of this license agreement and warranty.

Disclaimer of Warranty

THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED \"AS IS\"
AND WITHOUT WARRANTIES AS TO PERFORMANCE OR MERCHANTABILITY OR ANY
OTHER WARRANTIES WHETHER EXPRESSED OR IMPLIED.
NO WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE IS OFFERED.

Restrictions

You may use this software on a computer system only if you own the system
or have the permission of the owner.

Distribution

This is free software.
It is distributed under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation (http://www.fsf.org/).
The source code may be obtained
from the BOINC web site (http://boinc.berkeley.edu).

    </pre>
)";
if ($xml) {
    echo "</versions>\n";
} else {
    page_tail();
}
?>
