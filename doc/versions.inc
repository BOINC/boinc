<?php

function show_linux_info() {
    echo "
        <p>
        The current release is known to work with these Linux versions:
        <ul>
        <li> Fedora Core 7 and 8
        <li> Debian 4.1
        <li> Ubuntu 7.10 and 8.04
        <li> Mandriva, Sabayon, Mint, Gentoo, other Debian-based distributions
        </ul>
        <p>
        For other Linux versions,
        check if a BOINC package is offered by your Linux distribution.
        <ul>
        <li> Fedora 7 and higher: type
          <pre>yum install boinc-client boinc-manager</pre> as root.
        <li> Debian and Ubuntu: package names <b>boinc-client</b>,
            <b>boinc-manager</b>
            (more info <a href=\"http://wiki.debian.org/BOINC\">here</a>).
            Also <b>boinc-dev</b> (for project developers).
        <li> Gentoo: package name <b>sci-misc/boinc</b>
        </ul>
        <p>
        Alternatively, download the
        <a href=download_all.php>BOINC client for older Linux versions</a>.
        This doesn't have a graphical interface,
        but it should work on all Linux systems, both x86 and x64.
    ";
}

function type_text($type) {
    global $xml;
    if ($xml) {
        switch($type) {
        case "win_old":
            return "Single-mode Windows installer";
        case "win_new":
            return "Windows installer";
        case "bare_core":
            return "Core client only (command-line)";
        case "sea":
            return "Self-extracting archive";
        case "mac_advanced":
            return "Standard GUI";
        }
    } else {
        switch($type) {
        case "win_old":
            return "<a href=client_windows.php>Single-mode Windows installer</a>";
        case "win_new":
            return "<a href=win_install.php>Windows installer</a>";
        case "bare_core":
            return "<a href=bare_core.php>Core client only (command-line)</a>";
        case "sea":
            return "<a href=sea.php>Self-extracting archive</a>";
        case "mac_advanced":
            return "<a href=mac_advanced.php>Standard GUI</a>";
        }
    }
}

$w63= array(
    "num"=>"6.3.23",
    "status"=>"Development version",
    "file"=>"boinc_6.3.23_windows_intelx86.exe",
    "date"=>"14 Nov 2008",
    "type"=>"win_new",
);

$w6219= array(
    "num"=>"6.2.19",
    "status"=>"Recommended version",
    "file"=>"boinc_6.2.19_windows_intelx86.exe",
    "date"=>"22 Sep 2008",
    "type"=>"win_new",
);

$w51045= array(
    "num"=>"5.10.45",
    "status"=>"Older version",
    "file"=>"boinc_5.10.45_windows_intelx86.exe",
    "date"=>"5 Mar 2008",
    "type"=>"win_new",
);

$w63x64= array(
    "num"=>"6.3.23",
    "status"=>"Development version",
    "file"=>"boinc_6.3.23_windows_x86_64.exe",
    "date"=>"14 Nov 2008",
    "type"=>"win_new",
);

$w6219x64= array(
    "num"=>"6.2.19",
    "status"=>"Recommended version",
    "file"=>"boinc_6.2.19_windows_x86_64.exe",
    "date"=>"22 Sep 2008",
    "type"=>"win_new",
);

$w51045x64= array(
    "num"=>"5.10.45",
    "status"=>"Older version",
    "file"=>"boinc_5.10.45_windows_x86_64.exe",
    "date"=>"5 Mar 2008",
    "type"=>"win_new",
);

$w5816= array(
    "num"=>"5.8.16",
    "status"=>"Older version (also works on Windows 95 and NT 4.0)",
    "file"=>"boinc_5.8.16_windows_intelx86.exe",
    "date"=>"1 Mar 2007",
    "type"=>"win_new",
);

$m63 = array(
    "num"=>"6.3.23",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_6.3.23_macOSX_universal.zip",
    "date"=>"14 Nov 2008",
    "type"=>"mac_advanced",
);

$m6218 = array(
    "num"=>"6.2.18",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_6.2.18_macOSX_universal.zip",
    "date"=>"18 Aug 2008",
    "type"=>"mac_advanced",
);

$m51045 = array(
    "num"=>"5.10.45",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_5.10.45_macOSX_universal.zip",
    "date"=>"5 Mar 2008",
    "type"=>"mac_advanced",
);

$m5817 = array(
    "num"=>"5.8.17",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_5.8.17_macOSX_universal.zip",
    "date"=>"3 Apr 2007",
    "type"=>"mac_advanced",
);

$m63c = array(
    "num"=>"6.3.23",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_6.3.23_universal-apple-darwin.zip",
    "date"=>"14 Nov 2008",
    "type"=>"bare_core",
);

$m6218c = array(
    "num"=>"6.2.18",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_6.2.18_universal-apple-darwin.zip",
    "date"=>"18 Aug 2008",
    "type"=>"bare_core",
);

$m51045c = array(
    "num"=>"5.10.45",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_5.10.45_universal-apple-darwin.zip",
    "date"=>"5 Mar 2008",
    "type"=>"bare_core",
);

$m5817c = array(
    "num"=>"5.8.17",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_5.8.17_universal-apple-darwin.zip",
    "date"=>"3 Apr 2007",
    "type"=>"bare_core",
);

$l63 = array(
    "num"=>"6.3.21",
    "status"=>"Development version",
    "file"=>"boinc_compat_6.3.21_i686-pc-linux-gnu.sh",
    "date"=>"3 Nov 2008",
    "type"=>"sea",
);

$l6215 = array(
    "num"=>"6.2.15",
    "status"=>"Recommended version",
    "file"=>"boinc_compat_6.2.15_i686-pc-linux-gnu.sh",
    "date"=>"4 Aug 2008",
    "type"=>"sea",
);

$l51045 = array(
    "num"=>"5.10.45",
    "status"=>"Older version",
    "file"=>"boinc_5.10.45_i686-pc-linux-gnu.sh",
    "date"=>"5 Mar 2008",
    "type"=>"sea",
);

$l63ubuntu = array(
    "num"=>"6.3.21",
    "status"=>"Development version",
    "file"=>"boinc_6.3.21_i686-pc-linux-gnu.sh",
    "date"=>"3 Nov 2008",
    "type"=>"sea",
);

$l6215ubuntu = array(
    "num"=>"6.2.15",
    "status"=>"Recommended version",
    "file"=>"boinc_6.2.15_i686-pc-linux-gnu.sh",
    "date"=>"4 Aug 2008",
    "type"=>"sea",
);

$l51045ubuntu = array(
    "num"=>"5.10.45",
    "status"=>"Older version",
    "file"=>"boinc_ubuntu_5.10.45_i686-pc-linux-gnu.sh",
    "date"=>"5 Mar 2008",
    "type"=>"sea",
);

$l63ubuntux64 = array(
    "num"=>"6.3.21",
    "status"=>"Development version",
    "file"=>"boinc_6.3.21_x86_64-pc-linux-gnu.sh",
    "date"=>"3 Nov 2008",
    "type"=>"sea",
);

$l6215ubuntux64 = array(
    "num"=>"6.2.15",
    "status"=>"Recommended version",
    "file"=>"boinc_6.2.15_x86_64-pc-linux-gnu.sh",
    "date"=>"4 Aug 2008",
    "type"=>"sea",
);

$l51045ubuntux64 = array(
    "num"=>"5.10.45",
    "status"=>"Older version",
    "file"=>"boinc_ubuntu_5.10.45_x86_64-pc-linux-gnu.sh",
    "date"=>"5 Mar 2008",
    "type"=>"sea",
);

$l5816 = array(
    "num"=>"5.8.16",
    "status"=>"Older version (includes GUI)",
    "file"=>"boinc_5.8.16_i686-pc-linux-gnu.sh",
    "date"=>"7 March 2007",
    "type"=>"sea",
);

$windows = array(
    "name"=>"Windows",
    "description"=>"98/ME/2000/XP/Vista ",
    "dbname" => "windows_intelx86",
    "versions"=>array(
		$w63,
		$w6219,
		$w51045,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista",
    "dbname" => "windows_x86_64",
    "versions"=>array(
		$w63x64,
		$w6219x64,
        $w51045x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "description"=>"Version 10.3.9+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m63,
        $m63c,
        $m6218,
        $m6218c,
        $m51045,
        $m51045c,
        $m5817,
        $m5817c,
    )
);

$linuxcompat = array(
    "name"=>"Older Linux x86 and x64",
    "description"=>"For older Linux versions, both x86 and x64; command-line interface only",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l63,
		$l6215,
		$l51045,
        $l5816,
    )
);

$linux = array(
    "name"=>"Linux x86",
    "description"=>"For most current Linux/x86 versions; graphical and command-line interfaces",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l63ubuntu,
		$l6215ubuntu,
		$l51045ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=>"For most current Linux/x64 versions; graphical and command-line interfaces",
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
		$l63ubuntux64,
		$l6215ubuntux64,
		$l51045ubuntux64,
    )
);

$platforms = array(
    "win"=>$windows,
    "winx64"=>$windowsx64,
    "mac"=>$mac,
    "linux"=>$linux,
    "linuxx64"=>$linuxx64,
    "linuxcompat"=>$linuxcompat,
);

function is_dev($v) {
    return (strstr($v["status"], "Development") != null);
}

$url_base = "http://boincdl.ssl.berkeley.edu/dl/";
//switch(rand(0,5)) {
    //case 0: $url_base = "http://boinc.berkeley.edu/dl/"; break;
    //case 1: $url_base = "http://morel.mit.edu/download/boinc/dl/"; break;
    //case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    //case 3: $url_base = "http://einstein.astro.gla.ac.uk/download/boinc/dl/"; break;
    //case 4: $url_base = "http://albert.gravity.psu.edu/download/boinc/dl/"; break;
    //case 5: $url_base = "http://einstein.ligo.caltech.edu/download/boinc/dl/"; break;
    //case 6: $url_base = "http://einstein.phys.uwm.edu/download/boinc/dl/"; break;
//}

?>
