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
	"num"=>"7.4.27",
	"status"=>"Development version",
	"file"=>"boinc_7.4.27_windows_intelx86.exe",
	"vbox_file"=>"boinc_7.4.27_windows_intelx86_vbox.exe",
  "vbox_version"=>"4.3.12",
	"date"=>"5 Nov 2014",
	"type"=>"win_new",
);

$w72 = array(
	"num"=>"7.2.42",
	"status"=>"Recommended version",
	"file"=>"boinc_7.2.42_windows_intelx86.exe",
	"vbox_file"=>"boinc_7.2.42_windows_intelx86_vbox.exe",
  "vbox_version"=>"4.2.16",
	"date"=>"28 Feb 2014",
	"type"=>"win_new",
);

$w70 = array(
	"num"=>"7.0.64",
	"status"=>"Older version",
	"file"=>"boinc_7.0.64_windows_intelx86.exe",
	"date"=>"17 Apr 2013",
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
	"num"=>"7.4.27",
	"status"=>"Development version",
	"file"=>"boinc_7.4.27_windows_x86_64.exe",
	"vbox_file"=>"boinc_7.4.27_windows_x86_64_vbox.exe",
  "vbox_version"=>"4.3.12",
	"date"=>"5 Nov 2014",
	"type"=>"win_new",
);

$w72x64 = array(
	"num"=>"7.2.42",
	"status"=>"Recommended version",
	"file"=>"boinc_7.2.42_windows_x86_64.exe",
	"vbox_file"=>"boinc_7.2.42_windows_x86_64_vbox.exe",
  "vbox_version"=>"4.2.16",
	"date"=>"28 Feb 2014",
	"type"=>"win_new",
);

$w70x64 = array(
	"num"=>"7.0.64",
	"status"=>"Older version",
	"file"=>"boinc_7.0.64_windows_x86_64.exe",
	"date"=>"17 Apr 2013",
	"type"=>"win_new",
);

$m74dev = array(
	"num"=>"7.4.26",
	"status"=>"Development version (standard GUI)",
	"file"=>"boinc_7.4.26_macOSX_i686.zip",
	"date"=>"29 Oct 2014",
	"type"=>"mac_advanced",
);

$m72 = array(
	"num"=>"7.2.42",
	"status"=>"Recommended version (standard GUI)",
	"file"=>"boinc_7.2.42_macOSX_i686.zip",
	"date"=>"28 Feb 2014",
	"type"=>"mac_advanced",
);

$m70 = array(
	"num"=>"7.0.65",
	"status"=>"Older version (standard GUI)",
	"file"=>"boinc_7.0.65_macOSX_i686.zip",
	"date"=>"17 Apr 2013",
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
	"num"=>"7.4.26",
	"status"=>"Development version (Unix command-line version)",
	"file"=>"boinc_7.4.26_i686-apple-darwin.zip",
	"date"=>"29 Oct 2014",
	"type"=>"bare_core",
);

$m72c = array(
	"num"=>"7.2.42",
	"status"=>"Recommended version (Unix command-line version)",
	"file"=>"boinc_7.2.42_i686-apple-darwin.zip",
	"date"=>"28 Feb 2014",
	"type"=>"bare_core",
);

$m70c = array(
	"num"=>"7.0.65",
	"status"=>"Older version (Unix command-line version)",
	"file"=>"boinc_7.0.65_i686-apple-darwin.zip",
	"date"=>"17 Apr 2013",
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
	"num"=>"7.4.14",
	"status"=>"Development Version",
	"file"=>"boinc_7.4.14.apk",
	"date"=>"11 Aug 2014",
	"type"=>"apk",
);

$a73 = array(
	"num"=>"7.3.7",
	"status"=>"Recommended Version",
	"file"=>"boinc_7.3.7.apk",
	"date"=>"3 Mar 2014",
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
		  $w72,
		  $w70,
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
		  $w72x64,
		  $w70x64,
    )
);

$mac = array(
    "name"=>"Mac OS X (Intel only)",
    "description"=>"Version 10.4.0+",
    "dbname" => "x86_64-apple-darwin",
    "versions"=>array(
		  $m74dev,
      $m72,
		  $m70,
      $m74cdev,
		  $m72c,
		  $m70c,
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
		$l66,
	)
);

$androidarm = array(
	"name"=>"Android/ARM",
	"description"=>"Android version 2.3 and later<br>We recommend that, rather than downloading BOINC from here,</br>you get it from the Google Play Store or the Amazon app store (for Kindle Fire)",
	"dbname" => "arm-android-linux-gnu",
	"versions"=>array(
		$a74dev,
		$a73,
	)
);

$androidmips = array(
	"name"=>"Android/MIPS",
	"description"=>"Android version 2.3 and later<br>We recommend that, rather than downloading BOINC from here,</br>you get it from the Google Play Store or the Amazon app store (for Kindle Fire)",
	"dbname" => "mips-android-linux-gnu",
	"versions"=>array(
		$a74dev,
		$a73,
	)
);

$androidx86 = array(
	"name"=>"Android/x86",
	"description"=>"Android version 2.3 and later<br>We recommend that, rather than downloading BOINC from here,</br>you get it from the Google Play Store or the Amazon app store (for Kindle Fire)",
	"dbname" => "x86-android-linux-gnu",
	"versions"=>array(
		$a74dev,
		$a73,
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
    "androidarm"=>$androidarm,
    "androidmips"=>$androidmips,
    "androidx86"=>$androidx86,
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
