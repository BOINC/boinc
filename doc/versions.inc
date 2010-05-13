<?php

function show_linux_info() {
    echo "
        <p>
        ".tra("The current release is known to work with these Linux versions:")."
        <ul>
        <li> Fedora Core 7+
        <li> Debian 4.1+
        <li> Ubuntu 7.10+
        <li> Mandriva, Sabayon, Mint, Gentoo, other Debian-based distributions
        </ul>
        <p>
        ".tra("For other Linux versions, check if a BOINC package is offered by your Linux distribution.")."
        <ul>
        <li> Fedora 7+: type
          <pre>yum install boinc-client boinc-manager</pre> as root.
        <li> Debian and Ubuntu: package names <b>boinc-client</b>,
            <b>boinc-manager</b>
            (more info <a href=\"http://wiki.debian.org/BOINC\">here</a>).
            Also <b>boinc-dev</b> (for project developers).
        <li> Gentoo: package name <b>sci-misc/boinc</b>
        </ul>
        <p>
        ".tra("Alternatively, download the %1BOINC client for older Linux versions%2. This doesn't have a graphical interface, but it should work on all Linux systems, both x86 and x64.",
            "<a href=http://boinc.berkeley.edu/download_all.php>",
            "</a>"
        )
    ;
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

$w610dev = array(
	"num"=>"6.10.55",
	"status"=>"Development version",
	"file"=>"boinc_6.10.55_windows_intelx86.exe",
	"date"=>"13 May 2010",
	"type"=>"win_new",
);

$w610 = array(
	"num"=>"6.10.18",
	"status"=>"Recommended version",
	"file"=>"boinc_6.10.18_windows_intelx86.exe",
	"date"=>"9 Nov 2009",
	"type"=>"win_new",
);

$w66 = array(
    "num"=>"6.6.38",
	"status"=>"Older version (also works on Windows 98, Windows ME)",
    "file"=>"boinc_6.6.38_windows_intelx86.exe",
    "date"=>"28 Oct 2009",
    "type"=>"win_new",
);

$w5816 = array(
    "num"=>"5.8.16",
    "status"=>"Older version (also works on Windows 95 and NT 4.0)",
    "file"=>"boinc_5.8.16_windows_intelx86.exe",
    "date"=>"1 Mar 2007",
    "type"=>"win_new",
);

$w610x64dev = array(
	"num"=>"6.10.55",
	"status"=>"Development version",
	"file"=>"boinc_6.10.55_windows_x86_64.exe",
	"date"=>"13 May 2010",
	"type"=>"win_new",
);

$w610x64 = array(
	"num"=>"6.10.18",
	"status"=>"Recommended version",
	"file"=>"boinc_6.10.18_windows_x86_64.exe",
	"date"=>"9 Nov 2009",
	"type"=>"win_new",
);

$w66x64 = array(
    "num"=>"6.6.38",
	"status"=>"Older version",
    "file"=>"boinc_6.6.38_windows_x86_64.exe",
    "date"=>"28 Oct 2009",
    "type"=>"win_new",
);

$m610dev = array(
	"num"=>"6.10.55",
	"status"=>"Development version (standard GUI)",
	"file"=>"boinc_6.10.55_macOSX_universal.zip",
	"date"=>"13 May 2010",
	"type"=>"mac_advanced",
);

$m610 = array(
	"num"=>"6.10.51",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_6.10.51_macOSX_universal.zip",
	"date"=>"30 Apr 2010",
	"type"=>"mac_advanced",
);

$m610older = array(
    "num"=>"6.10.21",
    "status"=>"Older version (standard GUI)",
	"file"=>"boinc_6.10.21_macOSX_universal.zip",
	"date"=>"1 Dec 2009",
    "type"=>"mac_advanced",
);

$m66 = array(
    "num"=>"6.6.36",
    "status"=>"Older version (standard GUI) (works on OS 10.3.9 - OS 10.5.8)",
    "file"=>"boinc_6.6.36_macOSX_universal.zip",
    "date"=>"10 June 2009",
    "type"=>"mac_advanced",
);

$m610cdev = array(
	"num"=>"6.10.55",
	"status"=>"Development version (Unix command-line version)",
	"file"=>"boinc_6.10.55_universal-apple-darwin.zip",
	"date"=>"13 May 2010",
	"type"=>"bare_core",
);

$m610c = array(
	"num"=>"6.10.51",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_6.10.51_universal-apple-darwin.zip",
	"date"=>"30 Apr 2010",
	"type"=>"bare_core",
);

$m610colder = array(
	"num"=>"6.10.21",
	"status"=>"Older version (Unix command-line version)",
	"file"=>"boinc_6.10.21_universal-apple-darwin.zip",
	"date"=>"1 Dec 2009",
	"type"=>"bare_core",
);

$m66c = array(
    "num"=>"6.6.36",
    "status"=>"Older version (Unix command-line version) (works on OS 10.3.9 - OS 10.5.8)",
    "file"=>"boinc_6.6.36_universal-apple-darwin.zip",
    "date"=>"10 June 2009",
    "type"=>"bare_core",
);

$l66 = array(
	"num"=>"6.6.41",
	"status"=>"Recommended version",
	"file"=>"boinc_compat_6.6.41_i686-pc-linux-gnu.sh",
	"date"=>"22 Oct 2009",
	"type"=>"sea",
	);

$l64 = array(
    "num"=>"6.4.5",
    "status"=>"Older version",
    "file"=>"boinc_compat_6.4.5_i686-pc-linux-gnu.sh",
    "date"=>"9 Dec 2008",
    "type"=>"sea",
);

$l610ubuntudev = array(
	"num"=>"6.10.55",
	"status"=>"Development version",
	"file"=>"boinc_6.10.55_i686-pc-linux-gnu.sh",
	"date"=>"13 May 2010",
	"type"=>"sea",
);

$l610ubuntu = array(
	"num"=>"6.10.17",
	"status"=>"Recommended version",
	"file"=>"boinc_6.10.17_i686-pc-linux-gnu.sh",
	"date"=>"28 Oct 2009",
	"type"=>"sea",
);

$l66ubuntu = array(
	"num"=>"6.6.41",
	"status"=>"Older version",
	"file"=>"boinc_6.6.41_i686-pc-linux-gnu.sh",
	"date"=>"22 Oct 2009",
	"type"=>"sea",
);

$l610ubuntux64dev = array(
	"num"=>"6.10.55",
	"status"=>"Development version",
	"file"=>"boinc_6.10.55_x86_64-pc-linux-gnu.sh",
	"date"=>"13 May 2010",
	"type"=>"sea",
);

$l610ubuntux64 = array(
	"num"=>"6.10.17",
	"status"=>"Recommended version",
	"file"=>"boinc_6.10.17_x86_64-pc-linux-gnu.sh",
	"date"=>"28 Oct 2009",
	"type"=>"sea",
);

$l66ubuntux64 = array(
	"num"=>"6.6.41",
	"status"=>"Older version",
	"file"=>"boinc_6.6.41_x86_64-pc-linux-gnu.sh",
	"date"=>"22 Oct 2009",
	"type"=>"sea",
);

// platform descriptors.  For each one, "versions"
// lists versions in reverse time order.
// i.e. the first non-dev version is the current one

$windows = array(
    "name"=>"Windows",
    "description"=>"2000/XP/Vista/7",
    "dbname" => "windows_intelx86",
    "versions"=>array(
		$w610dev,
		$w610,
		$w66,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista/7",
    "dbname" => "windows_x86_64",
    "versions"=>array(
		$w610x64dev,
		$w610x64,
		$w66x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "description"=>"Version 10.4.0+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
		$m610dev,
		$m610,
		$m610cdev,
		$m610c,
        $m610older,
        $m610colder,
		$m66,
        $m66c,
    )
);

$linuxcompat = array(
    "name"=>"Older Linux x86 and x64",
    "description"=>"For older Linux versions, both x86 and x64; command-line interface only",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
        $l66,
        $l64,
    )
);

$linux = array(
    "name"=>"Linux x86",
    "description"=>"For most current Linux/x86 versions; graphical and command-line interfaces",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l610ubuntudev,
		$l610ubuntu,
		$l66ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=>"For most current Linux/x64 versions; graphical and command-line interfaces",
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
		$l610ubuntux64dev,
		$l610ubuntux64,
		$l66ubuntux64,
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

// note: rand() is inclusive
switch(rand(0,3)) {
//switch(0) {
    //case 0: $url_base = "http://boinc.berkeley.edu/dl/"; break;
    case 0: $url_base = "http://boincdl3.ssl.berkeley.edu/mirror/"; break;
    case 1: $url_base = "http://einstein.ligo.caltech.edu/download/boinc/dl/"; break;
    case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    //case 1: $url_base = "http://morel.mit.edu/download/boinc/dl/"; break;
    case 3: $url_base = "http://einstein-dl.phys.uwm.edu/download/boinc/dl/"; break;
    //case 4: $url_base = "http://einstein.astro.gla.ac.uk/download/boinc/dl/"; break;
    //case 4: $url_base = "http://albert.gravity.psu.edu/download/boinc/dl/"; break;
}

?>
