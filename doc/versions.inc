<?php

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

$w70dev = array(
	"num"=>"7.0.28",
	"status"=>"Development version",
	"file"=>"boinc_7.0.28_windows_intelx86.exe",
	"date"=>"21 May 2012",
	"type"=>"win_new",
);

$w70 = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.25_windows_intelx86.exe",
	"date"=>"5 Apr 2012",
	"type"=>"win_new",
);

$w612 = array(
	"num"=>"6.12.34",
	"status"=>"Older version",
	"file"=>"boinc_6.12.34_windows_intelx86.exe",
	"date"=>"28 July 2011",
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

$w70x64dev = array(
	"num"=>"7.0.28",
	"status"=>"Development version",
	"file"=>"boinc_7.0.28_windows_x86_64.exe",
	"date"=>"21 May 2012",
	"type"=>"win_new",
);

$w70x64 = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.25_windows_x86_64.exe",
	"date"=>"5 Apr 2012",
	"type"=>"win_new",
);

$w612x64 = array(
	"num"=>"6.12.34",
	"status"=>"Older version",
	"file"=>"boinc_6.12.34_windows_x86_64.exe",
	"date"=>"28 July 2011",
	"type"=>"win_new",
);

$m70dev = array(
	"num"=>"7.0.28",
	"status"=>"Development version (standard GUI)",
	"file"=>"boinc_7.0.28_macOSX_i686.zip",
	"date"=>"21 May 2012",
	"type"=>"mac_advanced",
);

$m70 = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_7.0.25_macOSX_i686.zip",
	"date"=>"5 Apr 2012",
	"type"=>"mac_advanced",
);

$m612 = array(
	"num"=>"6.12.35",
	"status"=>"Older version (standard GUI)",
	"file"=>"boinc_6.12.35_macOSX_universal.zip",
	"date"=>"11 Aug 2011",
	"type"=>"mac_advanced",
);

$m612_ppc = array(
	"num"=>"6.12.35",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_6.12.35_macOSX_universal.zip",
	"date"=>"11 Aug 2011",
	"type"=>"mac_advanced",
);

$m66 = array(
    "num"=>"6.6.36",
    "status"=>"Older version (standard GUI) (works on OS 10.3.9 - OS 10.5.8)",
    "file"=>"boinc_6.6.36_macOSX_universal.zip",
    "date"=>"10 June 2009",
    "type"=>"mac_advanced",
);

$m70c = array(
	"num"=>"7.0.28",
	"status"=>"Development version (Unix command-line version)",
	"file"=>"boinc_7.0.28_i686-apple-darwin.zip",
	"date"=>"21 May 2012",
	"type"=>"bare_core",
);

$m70c = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_7.0.25_i686-apple-darwin.zip",
	"date"=>"5 Apr 2012",
	"type"=>"bare_core",
);

$m612c = array(
	"num"=>"6.12.35",
	"status"=>"Older version (Unix command-line version)",
	"file"=>"boinc_6.12.35_universal-apple-darwin.zip",
	"date"=>"11 Aug 2011",
	"type"=>"bare_core",
);

$m612c_ppc = array(
	"num"=>"6.12.35",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_6.12.35_universal-apple-darwin.zip",
	"date"=>"11 Aug 2011",
	"type"=>"bare_core",
);

$m66c = array(
    "num"=>"6.6.36",
    "status"=>"Older version (Unix command-line version) (works on OS 10.3.9 - OS 10.5.8)",
    "file"=>"boinc_6.6.36_universal-apple-darwin.zip",
    "date"=>"10 June 2009",
    "type"=>"bare_core",
);

$l612 = array(
	"num"=>"6.12.22",
	"status"=>"Recommended version",
	"file"=>"boinc_compat_6.12.22_i686-pc-linux-gnu.sh",
	"date"=>"6 Apr 2011",
	"type"=>"sea",
);

$l66 = array(
	"num"=>"6.6.41",
	"status"=>"Older version",
	"file"=>"boinc_compat_6.6.41_i686-pc-linux-gnu.sh",
	"date"=>"22 Oct 2009",
	"type"=>"sea",
	);

$l70ubuntudev = array(
	"num"=>"7.0.28",
	"status"=>"Development version",
	"file"=>"boinc_7.0.28_i686-pc-linux-gnu.sh",
	"date"=>"21 May 2012",
	"type"=>"sea",
);

$l70ubuntu = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.25_i686-pc-linux-gnu.sh",
	"date"=>"5 Apr 2012",
	"type"=>"sea",
);

$l612ubuntu = array(
	"num"=>"6.12.34",
	"status"=>"Older version",
	"file"=>"boinc_6.12.34_i686-pc-linux-gnu.sh",
	"date"=>"28 July 2011",
	"type"=>"sea",
);

$l70ubuntux64 = array(
	"num"=>"7.0.28",
	"status"=>"Development version",
	"file"=>"boinc_7.0.28_x86_64-pc-linux-gnu.sh",
	"date"=>"21 May 2012",
	"type"=>"sea",
);

$l70ubuntux64 = array(
	"num"=>"7.0.25",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.25_x86_64-pc-linux-gnu.sh",
	"date"=>"5 Apr 2012",
	"type"=>"sea",
);

$l612ubuntux64 = array(
	"num"=>"6.12.34",
	"status"=>"Older version",
	"file"=>"boinc_6.12.34_x86_64-pc-linux-gnu.sh",
	"date"=>"28 July 2011",
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
		$w70dev,
		$w70,
		$w612,
		$w66,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista/7",
    "dbname" => "windows_x86_64",
    "versions"=>array(
		$w70x64dev,
		$w70x64,
		$w612x64,
    )
);

$mac = array(
    "name"=>"Mac OS X (Intel only)",
    "description"=>"Version 10.4.0+",
    "dbname" => "x86_64-apple-darwin",
    "versions"=>array(
		$m70dev,
		$m70cdev,
		$m70,
		$m70c,
		$m612,
		$m612c,
    )
);

$macppc = array(
    "name"=>"Mac OS X (PowerPC & Intel)",
    "description"=>"Version 10.4.0+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
		$m612_ppc,
		$m612c_ppc,
        $m66,
        $m66c,
    )
);

$linuxcompat = array(
    "name"=>"Older Linux x86 and x64",
    "description"=>"For older Linux versions, both x86 and x64; command-line interface only",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l612,
		$l66,
    )
);

$linux = array(
    "name"=>"Linux x86",
    "description"=>"Tested on the current Ubuntu release; may work on others.
    <br>If available, we recommend that you install a
    <a href=http://boinc.berkeley.edu/wiki/Installing_BOINC#Linux>
    distribution-specific package</a>
    instead.
    ",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l70ubuntudev,
		$l70ubuntu,
		$l612ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=>"Tested on the current Ubuntu release; may work on others.
    <br>If available, we recommend that you install a
    <a href=http://boinc.berkeley.edu/wiki/Installing_BOINC#Linux>
    distribution-specific package</a>
    instead.
    ",
    "versions"=>array(
		$l70ubuntux64dev,
		$l70ubuntux64,
		$l612ubuntux64,
    )
);

$platforms = array(
    "win"=>$windows,
    "winx64"=>$windowsx64,
    "mac"=>$mac,
    "macppc"=>$macppc,
    "linux"=>$linux,
    "linuxx64"=>$linuxx64,
    "linuxcompat"=>$linuxcompat,
);

function is_dev($v) {
    return (strstr($v["status"], "Development") != null);
}

// note: rand() is inclusive
//switch(rand(0,3)) {
switch(0) {
    case 0: $url_base = "http://boinc.berkeley.edu/dl/"; break;
    //case 0: $url_base = "http://boincdl3.ssl.berkeley.edu/mirror/"; break;
    //case 1: $url_base = "http://einstein.ligo.caltech.edu/download/boinc/dl/"; break;
    //case 2: $url_base = "http://einstein.aei.mpg.de/download/boinc/dl/"; break;
    //case 1: $url_base = "http://morel.mit.edu/download/boinc/dl/"; break;
    //case 3: $url_base = "http://einstein-dl.phys.uwm.edu/download/boinc/dl/"; break;
    //case 4: $url_base = "http://einstein.astro.gla.ac.uk/download/boinc/dl/"; break;
    //case 4: $url_base = "http://albert.gravity.psu.edu/download/boinc/dl/"; break;
}

?>
