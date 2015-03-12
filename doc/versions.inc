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
            return "Command-line client (no GUI)";
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
            return "<a href=bare_core.php>Command-line client (no GUI)</a>";
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

$w74dev = array(
    "num"=>"7.4.42",
    "status"=>"Development version",
    "file"=>"boinc_7.4.42_windows_intelx86.exe",
    "vbox_file"=>"boinc_7.4.42_windows_intelx86_vbox.exe",
    "vbox_version"=>"4.3.12",
    "date"=>"10 Mar 2015",
    "type"=>"win_new",
);

$w74 = array(
    "num"=>"7.4.36",
    "status"=>"Recommended version",
    "file"=>"boinc_7.4.36_windows_intelx86.exe",
    "vbox_file"=>"boinc_7.4.36_windows_intelx86_vbox.exe",
    "vbox_version"=>"4.3.12",
    "date"=>"12 Dec 2014",
    "type"=>"win_new",
);

$w72 = array(
    "num"=>"7.2.42",
    "status"=>"Older version",
    "file"=>"boinc_7.2.42_windows_intelx86.exe",
    "vbox_file"=>"boinc_7.2.42_windows_intelx86_vbox.exe",
    "vbox_version"=>"4.2.16",
    "date"=>"28 Feb 2014",
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

$w74x64dev = array(
    "num"=>"7.4.42",
    "status"=>"Development version",
    "file"=>"boinc_7.4.42_windows_x86_64.exe",
    "vbox_file"=>"boinc_7.4.42_windows_x86_64_vbox.exe",
    "vbox_version"=>"4.3.12",
    "date"=>"10 Mar 2015",
    "type"=>"win_new",
);

$w74x64 = array(
    "num"=>"7.4.36",
    "status"=>"Recommended version",
    "file"=>"boinc_7.4.36_windows_x86_64.exe",
    "vbox_file"=>"boinc_7.4.36_windows_x86_64_vbox.exe",
    "vbox_version"=>"4.3.12",
    "date"=>"12 Dec 2014",
    "type"=>"win_new",
);

$w72x64 = array(
    "num"=>"7.2.42",
    "status"=>"Older version",
    "file"=>"boinc_7.2.42_windows_x86_64.exe",
    "vbox_file"=>"boinc_7.2.42_windows_x86_64_vbox.exe",
    "vbox_version"=>"4.2.16",
    "date"=>"28 Feb 2014",
    "type"=>"win_new",
);

$m74dev = array(
	"num"=>"7.4.42",
	"status"=>"Development version (standard GUI)",
	"file"=>"boinc_7.4.42_macOSX_x86_64.zip",
	"date"=>"12 Mar 2015",
	"type"=>"mac_advanced",
);

$m74 = array(
	"num"=>"7.4.36",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_7.4.36_macOSX_i686.zip",
	"date"=>"12 Dec 2014",
	"type"=>"mac_advanced",
);

$m72 = array(
    "num"=>"7.2.42",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_7.2.42_macOSX_i686.zip",
    "date"=>"28 Feb 2014",
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

$m74cdev = array(
	"num"=>"7.4.42",
	"status"=>"Development version (Unix command-line version)",
	"file"=>"boinc_7.4.42_x86_64-apple-darwin.zip",
	"date"=>"12 Mar 2015",
	"type"=>"bare_core",
);

$m74c = array(
	"num"=>"7.4.36",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_7.4.36_i686-apple-darwin.zip",
	"date"=>"12 Dec 2014",
	"type"=>"bare_core",
);

$m72c = array(
    "num"=>"7.2.42",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_7.2.42_i686-apple-darwin.zip",
    "date"=>"28 Feb 2014",
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

$l74ubuntudev = array(
    "num"=>"7.4.22",
    "status"=>"Development version",
    "file"=>"boinc_7.4.22_i686-pc-linux-gnu.sh",
    "date"=>"17 Sept 2014",
    "type"=>"sea",
);

$l72ubuntu = array(
    "num"=>"7.2.42",
    "status"=>"Recommended version",
    "file"=>"boinc_7.2.42_i686-pc-linux-gnu.sh",
    "date"=>"28 Feb 2014",
    "type"=>"sea",
);

$l70ubuntu = array(
    "num"=>"7.0.65",
    "status"=>"Older version",
    "file"=>"boinc_7.0.65_i686-pc-linux-gnu.sh",
    "date"=>"17 Apr 2013",
    "type"=>"sea",
);

$l74ubuntux64dev = array(
    "num"=>"7.4.22",
    "status"=>"Development version",
    "file"=>"boinc_7.4.22_x86_64-pc-linux-gnu.sh",
    "date"=>"17 Sept 2014",
    "type"=>"sea",
);

$l72ubuntux64 = array(
    "num"=>"7.2.42",
    "status"=>"Recommended version",
    "file"=>"boinc_7.2.42_x86_64-pc-linux-gnu.sh",
    "date"=>"28 Feb 2014",
    "type"=>"sea",
);

$l70ubuntux64 = array(
    "num"=>"7.0.65",
    "status"=>"Older version",
    "file"=>"boinc_7.0.65_x86_64-pc-linux-gnu.sh",
    "date"=>"17 Apr 2013",
    "type"=>"sea",
);

$a74dev = array(
    "num"=>"7.4.31",
    "status"=>"Development Version",
    "file"=>"boinc_7.4.31.apk",
    "date"=>"26 Nov 2014",
    "type"=>"apk",
);

$a74 = array(
    "num"=>"7.4.41",
    "status"=>"Recommended Version",
    "file"=>"boinc_7.4.41.apk",
    "date"=>"22 Jan 2015",
    "type"=>"apk",
);

// platform descriptors.  For each one, "versions"
// lists versions in reverse time order.
// i.e. the first non-dev version is the current one

$windows = array(
    "name"=>"Windows 32-bit",
    "description"=>"XP/Vista/7/8/8.1",
    "dbname" => "windows_intelx86",
    "versions"=>array(
          $w74dev,
          $w74,
          $w72,
          $w66,
          $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista/7/8/8.1",
    "dbname" => "windows_x86_64",
    "versions"=>array(
          $w74x64dev,
          $w74x64,
          $w72x64,
    )
);

$mac = array(
    "name"=>"Mac OS X (64-bit Intel)",
    "description"=>"Version 10.5.0+",
    "dbname" => "x86_64-apple-darwin",
    "versions"=>array(
          $m74dev,
          $m74,
          $m74cdev,
          $m74c,
    )
);

$mac32 = array(
    "name"=>"Mac OS X (32-bit or 64-bit Intel)",
    "description"=>"Version 10.4.0+",
    "dbname" => "x86_64-apple-darwin",
    "versions"=>array(
          $m72,
          $m72c,
    )
);

$macppc = array(
    "name"=>"Mac OS X (Power PC or 32-bit Intel)",
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
          $l74ubuntudev,
          $l72ubuntu,
          $l70ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=> linux_info(),
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
          $l74ubuntux64dev,
          $l72ubuntux64,
          $l70ubuntux64,
    )
);

$linuxcompat = array(
    "name"=>"Older Linux x86 and x64",
    "description"=>"For older Linux versions, both x86 and x64; command-line interface only",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
        $l612,
    )
);

$android = array(
    "name"=>"Android",
    "description"=>"Android version 4.1 and later for ARM, MIPS, or x86<br>We recommend that, rather than downloading BOINC from here,</br>you get it from the Google Play Store or the Amazon app store (for Kindle Fire)",
    "dbname" => "arm-android-linux-gnu",
    "versions"=>array(
        $a74,
    )
);

$platforms = array(
    "win"=>$windows,
    "winx64"=>$windowsx64,
    "mac"=>$mac,
    "mac32"=>$mac32,
    "macppc"=>$macppc,
    "linuxx64"=>$linuxx64,
    "linux"=>$linux,
    "linuxcompat"=>$linuxcompat,
    "android"=>$android,
);

function is_dev($v) {
    return (strstr($v["status"], "Development") != null);
}

function latest_version($p) {
    $dev = false;
    foreach ($p['versions'] as $i=>$v) {
        if (!$dev && is_dev($v)) continue;
        return $v;
    }
    foreach ($p['versions'] as $i=>$v) {
        return $v;
    }
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
