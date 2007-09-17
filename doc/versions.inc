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

$w51020= array(
    "num"=>"5.10.20",
    "status"=>"Recommended version (Win98+, Win2000+)",
    "file"=>"boinc_5.10.20_windows_intelx86.exe",
    "date"=>"23 Aug 2007",
    "type"=>"win_new",
);

$w51020x64= array(
    "num"=>"5.10.20",
    "status"=>"Recommended version (WinXP64+)",
    "file"=>"boinc_5.10.20_windows_x86_64.exe",
    "date"=>"23 Aug 2007",
    "type"=>"win_new",
);

$w5816= array(
    "num"=>"5.8.16",
    "status"=>"Recommended version (Win95+, WinNT40+)",
    "file"=>"boinc_5.8.16_windows_intelx86.exe",
    "date"=>"1 Mar 2007",
    "type"=>"win_new",
);

$w5411= array(
    "num"=>"5.4.11",
    "status"=>"Older version",
    "file"=>"boinc_5.4.11_windows_intelx86.exe",
    "date"=>"2 Aug 2006",
    "type"=>"win_new",
);

$m51020 = array(
    "num"=>"5.10.20",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.10.20_macOSX_universal.zip",
    "date"=>"23 Aug 2007",
    "type"=>"mac_advanced",
);

$m5817 = array(
    "num"=>"5.8.17",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.8.17_macOSX_universal.zip",
    "date"=>"3 Apr 2007",
    "type"=>"mac_advanced",
);

$m549 = array(
    "num"=>"5.4.9",
    "status"=>"Older version (standard GUI)",
    "file"=>"boinc_5.4.9_macOSX_universal.zip",
    "date"=>"5 May 2006",
    "type"=>"mac_advanced",
);

$m51020c = array(
    "num"=>"5.10.20",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_5.10.20_universal-apple-darwin.zip",
    "date"=>"23 Aug 2007",
    "type"=>"bare_core",
);

$m5817c = array(
    "num"=>"5.8.17",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_5.8.17_universal-apple-darwin.zip",
    "date"=>"3 Apr 2007",
    "type"=>"bare_core",
);

$m549c = array(
    "num"=>"5.4.9",
    "status"=>"Older version (Unix command-line version)",
    "file"=>"boinc_5.4.9_universal-apple-darwin.zip",
    "date"=>"5 May 2006",
    "type"=>"bare_core",
);

$l5108 = array(
    "num"=>"5.10.8",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.10.8_i686-pc-linux-gnu.sh",
    "date"=>"24 June 2007",
    "type"=>"sea",
);

$l5108x64 = array(
    "num"=>"5.10.8",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.10.8_x86_64-pc-linux-gnu.sh",
    "date"=>"24 June 2007",
    "type"=>"sea",
);

$l5816 = array(
    "num"=>"5.8.16",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.8.16_i686-pc-linux-gnu.sh",
    "date"=>"7 March 2007",
    "type"=>"sea",
);

$l5411 = array(
    "num"=>"5.4.11",
    "status"=>"Recommended version",
    "file"=>"boinc_5.4.11_i686-pc-linux-gnu.sh",
    "date"=>"12 December 2006",
    "type"=>"sea",
);

$windows = array(
    "name"=>"Windows",
    "dbname" => "windows_intelx86",
    "versions"=>array(
        $w51020,
        $w5816,
        $w5411,
    )
);

$windowsx64 = array(
    "name"=>"Windows x64",
    "dbname" => "windows_x86_64",
    "versions"=>array(
        $w51020x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m51020,
        $m51020c,
        $m5817,
        $m5817c,
        $m549,
        $m549c,
    )
);

$linux = array(
    "name"=>"Linux/x86",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
        $l5108,
        $l5816,
        $l5411,
    )
);

$linuxx64 = array(
    "name"=>"Linux/x64",
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
        $l5108x64,
    )
);

$platforms = array(
    "win"=>$windows,
    "winx64"=>$windowsx64,
    "mac"=>$mac,
    "linux"=>$linux,
    "linuxx64"=>$linuxx64,
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
