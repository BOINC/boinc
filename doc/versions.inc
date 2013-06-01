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
        case "apk":
            return "Android Installer";
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
		case "apk":
			return "TBD";
		}
    }
}

function linux_info() {
	return "Tested on the current Ubuntu distribution; may work on others.
    <br>If available, we recommend that you install a
    <a href=http://boinc.berkeley.edu/wiki/Installing_BOINC#Linux>
    distribution-specific package</a>
    instead.  ";
}

$w72dev = array(
	"num"=>"7.1.3",
	"status"=>"Development version",
	"file"=>"boinc_7.1.3_windows_intelx86.exe",
	"date"=>"1 June 2013",
	"type"=>"win_new",
);

$w70 = array(
	"num"=>"7.0.64",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.64_windows_intelx86.exe",
	"date"=>"17 Apr 2013",
	"type"=>"win_new",
);

$w70old = array(
	"num"=>"7.0.28",
	"status"=>"Older version",
	"file"=>"boinc_7.0.28_windows_intelx86.exe",
	"date"=>"21 May 2012",
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

$w72x64dev = array(
	"num"=>"7.1.3",
	"status"=>"Development version",
	"file"=>"boinc_7.1.3_windows_x86_64.exe",
	"date"=>"1 June 2013",
	"type"=>"win_new",
);

$w70x64 = array(
	"num"=>"7.0.64",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.64_windows_x86_64.exe",
	"date"=>"17 Apr 2013",
	"type"=>"win_new",
);

$w70x64old = array(
	"num"=>"7.0.28",
	"status"=>"Older version",
	"file"=>"boinc_7.0.28_windows_x86_64.exe",
	"date"=>"21 May 2012",
	"type"=>"win_new",
);

$m72dev = array(
	"num"=>"7.1.3",
	"status"=>"Development version (standard GUI)",
	"file"=>"boinc_7.1.3_macOSX_i686.zip",
	"date"=>"1 June 2013",
	"type"=>"mac_advanced",
);

$m70 = array(
	"num"=>"7.0.65",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_7.0.65_macOSX_i686.zip",
	"date"=>"17 Apr 2013",
	"type"=>"mac_advanced",
);

$m70old = array(
	"num"=>"7.0.31",
	"status"=>"Older version (standard GUI)",
	"file"=>"boinc_7.0.31_macOSX_i686.zip",
	"date"=>"7 July 2012",
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

$m72cdev = array(
	"num"=>"7.1.3",
	"status"=>"Development version (Unix command-line version)",
	"file"=>"boinc_7.1.3_i686-apple-darwin.zip",
	"date"=>"1 June 2013",
	"type"=>"bare_core",
);

$m70c = array(
	"num"=>"7.0.65",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_7.0.65_i686-apple-darwin.zip",
	"date"=>"17 Apr 2013",
	"type"=>"bare_core",
);

$m70cold = array(
	"num"=>"7.0.31",
	"status"=>"Older version (Unix command-line version)",
	"file"=>"boinc_7.0.31_i686-apple-darwin.zip",
	"date"=>"29 July 2012",
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

$l72ubuntudev = array(
	"num"=>"7.1.3",
	"status"=>"Development version",
	"file"=>"boinc_7.1.3_i686-pc-linux-gnu.sh",
	"date"=>"1 June 2013",
	"type"=>"sea",
);

$l70ubuntu = array(
	"num"=>"7.0.65",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.65_i686-pc-linux-gnu.sh",
	"date"=>"17 Apr 2013",
	"type"=>"sea",
);

$l70ubuntuold = array(
	"num"=>"7.0.28",
	"status"=>"Older version",
	"file"=>"boinc_7.0.28_i686-pc-linux-gnu.sh",
	"date"=>"21 May 2012",
	"type"=>"sea",
);

$l72ubuntux64dev = array(
	"num"=>"7.1.3",
	"status"=>"Development version",
	"file"=>"boinc_7.1.3_x86_64-pc-linux-gnu.sh",
	"date"=>"1 June 2013",
	"type"=>"sea",
);

$l70ubuntux64 = array(
	"num"=>"7.0.65",
	"status"=>"Recommended version",
	"file"=>"boinc_7.0.65_x86_64-pc-linux-gnu.sh",
	"date"=>"17 Apr 2013",
	"type"=>"sea",
);

$l70ubuntux64old = array(
	"num"=>"7.0.28",
	"status"=>"Older version",
	"file"=>"boinc_7.0.28_x86_64-pc-linux-gnu.sh",
	"date"=>"21 May 2012",
	"type"=>"sea",
);

$adev = array(
	"num"=>"7.0.58",
	"status"=>"Development version",
	"file"=>"boinc_7.0.58_arm-android-linux-gnu.apk",
	"date"=>"21 Mar 2013",
	"type"=>"apk",
);

// platform descriptors.  For each one, "versions"
// lists versions in reverse time order.
// i.e. the first non-dev version is the current one

$windows = array(
    "name"=>"Windows",
    "description"=>"2000/XP/Vista/7",
    "dbname" => "windows_intelx86",
    "versions"=>array(
		$w72dev,
		$w70,
		$w70old,
		$w66,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista/7",
    "dbname" => "windows_x86_64",
    "versions"=>array(
		$w72x64dev,
		$w70x64,
		$w70x64old,
		$w612x64,
    )
);

$mac = array(
    "name"=>"Mac OS X (Intel only)",
    "description"=>"Version 10.4.0+",
    "dbname" => "x86_64-apple-darwin",
    "versions"=>array(
		$m72dev,
		$m70,
		$m70old,
		$m70c,
		$m70cold,
    )
);

$macppc = array(
    "name"=>"Mac OS X (PowerPC and x86)",
    "description"=>"Version 10.4.0+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
		$m612_ppc,
		$m612c_ppc,
        $m66,
        $m66c,
    )
);

$linux = array(
    "name"=>"Linux x86",
    "description"=> linux_info(),
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
		$l72ubuntudev,
		$l70ubuntu,
		$l70ubuntuold,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=> linux_info(),
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
		$l72ubuntux64dev,
		$l70ubuntux64,
		$l70ubuntux64old,
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

$android = array(
	"name"=>"Android",
	"description"=>"For Android 2.3+ devices",
	"dbname" => "arm-android-linux-gnu",
	"versions"=>array(
		$adev,
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
	"android"=>$android,
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
