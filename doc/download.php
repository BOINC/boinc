<?php

require_once("docutil.php");

function version_start($num, $date, $xml) {
    if ($xml) return;
    list_start();
    list_bar("Version $num (released $date)");
    list_heading("Platform", "click to download", "MD5 checksum of download file");
}

function version($platform, $filename, $xml) {
    $path = "dl/$filename";
    $dlink = "<a href=$path>Download</a>";
    $md = md5_file($path);
    if ($xml) {
        echo "
<version>
    <platform>$platform</platform>
    <url>http://boinc.berkeley.edu/$path</url>
    <filename>$filename</filename>
    <md5>$md</md5>
</version>
";
    } else {
        list_item($platform, $dlink, $md);
    }
}

function version_end($xml) {
    if ($xml) return;
    list_end();
}

function show_464($xml=false) {
    version_start("4.64", "1 Feb 2005", $xml);
    version("Windows", "boinc_4.64_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_463($xml=false) {
    version_start("4.63", "1 Feb 2005", $xml);
    version("Windows", "boinc_4.63_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_462($xml=false) {
    version_start("4.62", "24 Jan 2005", $xml);
    version("Windows", "boinc_4.62_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.62_i686-pc-linux-gnu.sh", $xml);
    version("Solaris/SPARC", "boinc_4.62_sparc-sun-solaris2.7.sh", $xml);
    version_end($xml);
}

function show_460($xml=false) {
    version_start("4.60", "20 Jan 2005", $xml);
    version("Windows", "boinc_4.60_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_459($xml=false) {
    version_start("4.59", "13 Jan 2005", $xml);
    version("Windows", "boinc_4.59_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_458($xml=false) {
    version_start("4.58", "9 Jan 2005", $xml);
    version("Windows", "boinc_4.58_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_457($xml=false) {
    version_start("4.57", "23 Dec 2004", $xml);
    version("Windows", "boinc_4.57_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_456($xml=false) {
    version_start("4.56", "23 Nov 2004", $xml);
    version("Windows", "boinc_4.56_windows_intelx86.exe", $xml);
    version_end($xml);
}

function show_419($xml=false) {
    version_start("4.19", "25 Jan 2005", $xml);
    version("Windows", "boinc_4.19_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.19_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.19_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.19_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_418($xml=false) {
    version_start("4.18", "24 Jan 2005", $xml);
    version("Windows", "boinc_4.18_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.18_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.18_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.18_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_417($xml=false) {
    version_start("4.17", "23 Jan 2005", $xml);
    version("Windows", "boinc_4.17_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.17_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.17_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.17_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_416($xml=false) {
    version_start("4.16", "19 Jan 2005", $xml);
    version("Windows", "boinc_4.16_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.16_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.16_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.16_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_415($xml=false) {
    version_start("4.15", "18 Jan 2005", $xml);
    version("Windows", "boinc_4.15_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.15_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.15_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.15_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_414($xml=false) {
    version_start("4.14", "13 Jan 2005", $xml);
    version("Windows", "boinc_4.14_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.14_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.14_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.14_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_413($xml=false) {
    version_start("4.13", "13 Oct 2004", $xml);
    version("Windows", "boinc_4.13_windows_intelx86.exe", $xml);
    version("Linux/x86", "boinc_4.13_i686-pc-linux-gnu.gz", $xml);
    version("Mac OS X", "boinc_4.13_powerpc-apple-darwin.gz", $xml);
    version("Solaris/SPARC", "boinc_4.13_sparc-sun-solaris2.7.gz", $xml);
    version_end($xml);
}

function show_stable($xml) {
    show_419($xml);
}

function show_dev($xml) {
    show_464($xml);
    show_462($xml);
}

if ($_GET["no_header"]) {
    show_stable(false);
    if ($_GET["dev"]) {
        show_dev(false);
    }
    exit();
}

if ($_GET["xml"]) {
    header ("Content-Type: text/xml");
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>
<core_versions>
    ";
    echo "<stable_versions/>\n";
    show_stable(true);
    echo "<unstable_versions/>\n";
    show_dev(true);
    echo "</core_versions>\n";
    exit();
}

page_head("Download BOINC client software");

echo "
<h2>System requirements</h2>
<p>
There are no specific hardware requirements
(CPU speed, memory, disk space, etc.).
However, these factors may limit the amount or type
of work that is sent to your computer.

<h2>Current version</h2>
";
show_stable(false);
echo "
    After the download is finished:
    <ul>
    <li><b>Windows</b>:
        open the downloaded file.
        This will install BOINC on your computer.
    <li><b>Macintosh OS X, Unix and Linux</b>:
        Use gunzip to uncompress if your browser has not done it for you.
        Then chmod +x the executable and run it.
    </ul>
    <p>

    If your computer is not one of the above types, you can
    <ul>
    <li> <a href=anonymous_platform.php>download and compile the BOINC software yourself</a> or
    <li> <a href=download_other.php>download from a third-party site</a>.
    </ul>
    <p>
    BOINC can be customized for
    <a href=http://boinc.berkeley.edu/language.php>languages other than English</a>

    <p>
    <a href=download.php?dev=1>Show development versions also</a>.
    <p>
    Get data in <a href=download.php?xml=1>XML format</a>.
";

if ($_GET["dev"]) {
    echo "
        <h2>Development versions (latest features, possibly buggy)</h2>
    ";
    show_dev(false);
}


page_tail();

?>
