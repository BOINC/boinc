<?php

require_once("docutil.php");

page_head("Download BOINC client software");

function version_start($num, $date) {
    list_start();
    list_bar("Version $num (released $date)");
    list_heading("Platform", "click to download", "MD5 checksum");
}

function version($platform, $filename) {
    $path = "dl/$filename";
    $dlink = "<a href=$path>Download</a>";
    $md = md5($path);
    list_item($platform, $dlink, $md);
}

function version_end() {
    list_end();
}

function show_455() {
    version_start("4.55", "13 Nov 2004");
    version("Windows", "boinc_4.55_windows_intelx86.exe");
    version_end();
}

function show_413() {
    version_start("4.13", "13 Oct 2004");
    version("Windows", "boinc_4.13_windows_intelx86.exe");
    version("Linux/x86", "boinc_4.13_i686-pc-linux-gnu.gz");
    version("Mac OS X", "boinc_4.13_powerpc-apple-darwin.gz");
    version("Solaris/SPARC", "boinc_4.13_sparc-sun-solaris2.7.gz");
    version_end();
}

echo "
<p>
Learn more about BOINC
<a href=http://boinc.berkeley.edu/participate.php>here</a>.


<h2>System requirements</h2>
The BOINC core client is available for the following platforms:
<ul>
<li> Windows (95 and up)
<li> Linux/x86
<li> Solaris/SPARC
<li> Mac OS X
</ul>
<p>
There are no specific hardware requirements
(CPU speed, RAM, disk space, etc.).
However, these factors may limit the amount or type
of work that is sent to your computer.
Each 'work unit' has minimum RAM and disk requirements,
and a deadline for completion of its computation.
A BOINC project won't send a work unit to a computer that can't handle it.
<p>
<h2>Stable version (recommended)</h2>
";
show_413();

echo "
    If your computer is not one of the above types, you can
    <ul>
    <li> <a href=anonymous_platform.php>download and compile the BOINC software yourself</a> or
    <li> <a href=download_other.php>download from a third-party site</a>.
    </ul>
    <p>
    BOINC can be customized for
    <a href=http://boinc.berkeley.edu/language.php>languages other than English</a>
";
echo "
<h2>Development version (latest features, possibly buggy)</h2>
";
show_455();

//echo"<h2>Old versions</h2>\n";
//show_405();


page_tail();

?>
