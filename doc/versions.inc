<?php

function show_linux_info() {
    echo "
        <p>
        The current release is known to work with these Linux versions:
        <ul>
        <li> Fedora Core 7, 8, and 9
        <li> Debian 4.1
        <li> Ubuntu 7.10, 8.04, and 8.10
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

$w610dev= array(
    "num"=>"6.10.15",
    "status"=>"Development version",
    "file"=>"boinc_6.10.15_windows_intelx86.exe",
    "date"=>"19 Oct 2009",
    "type"=>"win_new",
);

$w66= array(
    "num"=>"6.6.38",
    "status"=>"Recommended version",
    "file"=>"boinc_6.6.38_windows_intelx86.exe",
    "date"=>"24 Sept 2009",
    "type"=>"win_new",
);

$w64= array(
    "num"=>"6.4.7",
    "status"=>"Older version",
    "file"=>"boinc_6.4.7_windows_intelx86.exe",
    "date"=>"3 Mar 2009",
    "type"=>"win_new",
);

$w610x64dev= array(
    "num"=>"6.10.15",
    "status"=>"Development version",
    "file"=>"boinc_6.10.15_windows_x86_64.exe",
    "date"=>"19 Oct 2009",
    "type"=>"win_new",
);

$w66x64= array(
    "num"=>"6.6.38",
    "status"=>"Recommended version",
    "file"=>"boinc_6.6.38_windows_x86_64.exe",
    "date"=>"24 Sept 2009",
    "type"=>"win_new",
);

$w64x64= array(
    "num"=>"6.4.7",
    "status"=>"Older version",
    "file"=>"boinc_6.4.7_windows_x86_64.exe",
    "date"=>"3 Mar 2009",
    "type"=>"win_new",
);

$w5816= array(
    "num"=>"5.8.16",
    "status"=>"Older version (also works on Windows 95 and NT 4.0)",
    "file"=>"boinc_5.8.16_windows_intelx86.exe",
    "date"=>"1 Mar 2007",
    "type"=>"win_new",
);

$m610dev = array(
    "num"=>"6.10.14",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_6.10.14_macOSX_universal.zip",
    "date"=>"19 Oct 2009",
    "type"=>"mac_advanced",
);

$m66 = array(
    "num"=>"6.6.36",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_6.6.36_macOSX_universal.zip",
    "date"=>"10 June 2009",
    "type"=>"mac_advanced",
);

$m6218 = array(
    "num"=>"6.2.18",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_6.2.18_macOSX_universal.zip",
    "date"=>"18 Aug 2008",
    "type"=>"mac_advanced",
);

$m610cdev = array(
    "num"=>"6.10.14",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_6.10.14_universal-apple-darwin.zip",
    "date"=>"19 Oct 2009",
    "type"=>"bare_core",
);

$m66c = array(
    "num"=>"6.6.36",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_6.6.36_universal-apple-darwin.zip",
    "date"=>"10 June 2009",
    "type"=>"bare_core",
);

$m6218c = array(
    "num"=>"6.2.18",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_6.2.18_universal-apple-darwin.zip",
    "date"=>"18 Aug 2008",
    "type"=>"bare_core",
);

$l66 = array(
	"num"=>"6.6.40",
	"status"=>"Recommended version",
	"file"=>"boinc_compat_6.6.40_i686-pc-linux-gnu.sh",
	"date"=>"21 Sept 2009",
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
	"num"=>"6.10.13",
	"status"=>"Development version",
	"file"=>"boinc_6.10.13_i686-pc-linux-gnu.sh",
	"date"=>"6 Oct 2009",
	"type"=>"sea",
	);

$l66ubuntu = array(
	"num"=>"6.6.40",
	"status"=>"Recommended version",
	"file"=>"boinc_6.6.40_i686-pc-linux-gnu.sh",
	"date"=>"21 Sept 2009",
	"type"=>"sea",
	);

$l64ubuntu = array(
    "num"=>"6.4.5",
    "status"=>"Older version",
    "file"=>"boinc_6.4.5_i686-pc-linux-gnu.sh",
    "date"=>"9 Dec 2008",
    "type"=>"sea",
);

$l610ubuntux64dev = array(
	"num"=>"6.10.13",
	"status"=>"Development version",
	"file"=>"boinc_6.10.13_x86_64-pc-linux-gnu.sh",
	"date"=>"6 Oct 2009",
	"type"=>"sea",
	);

$l66ubuntux64 = array(
	"num"=>"6.6.40",
	"status"=>"Recommended version",
	"file"=>"boinc_6.6.40_x86_64-pc-linux-gnu.sh",
	"date"=>"21 Sept 2009",
	"type"=>"sea",
	);

$l64ubuntux64 = array(
    "num"=>"6.4.5",
    "status"=>"Older version",
    "file"=>"boinc_6.4.5_x86_64-pc-linux-gnu.sh",
    "date"=>"9 Dec 2008",
    "type"=>"sea",
);

// platform descriptors.  For each one, "versions"
// lists versions in reverse time order.
// i.e. the first non-dev version is the current one

$windows = array(
    "name"=>"Windows",
    "description"=>"98/ME/2000/XP/Vista ",
    "dbname" => "windows_intelx86",
    "versions"=>array(
        $w610dev,
        $w66,
        $w64,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista",
    "dbname" => "windows_x86_64",
    "versions"=>array(
        $w610x64dev,
        $w66x64,
        $w64x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "description"=>"Version 10.3.9+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m610dev,
        $m610cdev,
        $m66,
        $m66c,
        $m6218,
        $m6218c,
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
		$l66ubuntu,
		$l64ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=>"For most current Linux/x64 versions; graphical and command-line interfaces",
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
		$l610ubuntux64dev,
		$l66ubuntux64,
		$l64ubuntux64,
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

//$url_base = "http://boincdl.ssl.berkeley.edu/dl/";
//$url_base = "http://mirror.worldcommunitygrid.org/mirror/";

// note: rand() is inclusive
//switch(rand(0,2)) {
switch(0) {
    case 0: $url_base = "http://boincdl3.ssl.berkeley.edu/mirror/"; break;
    case 1: $url_base = "http://einstein.ligo.caltech.edu/download/boinc/dl/"; break;
    case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    //case 0: $url_base = "http://boinc.berkeley.edu/dl/"; break;
    //case 1: $url_base = "http://morel.mit.edu/download/boinc/dl/"; break;
    //case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    //case 3: $url_base = "http://einstein.astro.gla.ac.uk/download/boinc/dl/"; break;
    //case 4: $url_base = "http://albert.gravity.psu.edu/download/boinc/dl/"; break;
    //case 6: $url_base = "http://einstein.phys.uwm.edu/download/boinc/dl/"; break;
}

?>
