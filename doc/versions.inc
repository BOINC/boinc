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

$w51044= array(
    "num"=>"5.10.44",
    "status"=>"Development version",
    "file"=>"boinc_5.10.44_windows_intelx86.exe",
    "date"=>"29 Feb 2008",
    "type"=>"win_new",
);

$w51030= array(
    "num"=>"5.10.30",
    "status"=>"Recommended version",
    "file"=>"boinc_5.10.30_windows_intelx86.exe",
    "date"=>"13 Nov 2007",
    "type"=>"win_new",
);

$w51044x64= array(
    "num"=>"5.10.44",
    "status"=>"Development version",
    "file"=>"boinc_5.10.44_windows_x86_64.exe",
    "date"=>"29 Feb 2008",
    "type"=>"win_new",
);

$w51030x64= array(
    "num"=>"5.10.30",
    "status"=>"Recommended version",
    "file"=>"boinc_5.10.30_windows_x86_64.exe",
    "date"=>"13 Nov 2007",
    "type"=>"win_new",
);

$w5816= array(
    "num"=>"5.8.16",
    "status"=>"Older version (also works on Windows 95 and NT 4.0)",
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

$m51044 = array(
    "num"=>"5.10.44",
    "status"=>"Development version (standard GUI)",
    "file"=>"boinc_5.10.44_macOSX_universal.zip",
    "date"=>"1 Mar 2008",
    "type"=>"mac_advanced",
);

$m51034 = array(
    "num"=>"5.10.34",
    "status"=>"Recommended version (standard GUI)",
    "file"=>"boinc_5.10.34_macOSX_universal.zip",
    "date"=>"8 Jan 2008",
    "type"=>"mac_advanced",
);

$m5817 = array(
    "num"=>"5.8.17",
    "status"=>"Older version (standard GUI)",
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

$m51044c = array(
    "num"=>"5.10.44",
    "status"=>"Development version (Unix command-line version)",
    "file"=>"boinc_5.10.44_universal-apple-darwin.zip",
    "date"=>"1 Mar 2008",
    "type"=>"bare_core",
);

$m51034c = array(
    "num"=>"5.10.34",
    "status"=>"Recommended version (Unix command-line version)",
    "file"=>"boinc_5.10.34_universal-apple-darwin.zip",
    "date"=>"8 Jan 2008",
    "type"=>"bare_core",
);

$m5817c = array(
    "num"=>"5.8.17",
    "status"=>"Older version (Unix command-line version)",
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

$l51043 = array(
    "num"=>"5.10.43",
    "status"=>"Development version",
    "file"=>"boinc_5.10.43_i686-pc-linux-gnu.sh",
    "date"=>"26 Feb 2008",
    "type"=>"sea",
);

$l51028 = array(
    "num"=>"5.10.28",
    "status"=>"Recommended version",
    "file"=>"boinc_5.10.28_i686-pc-linux-gnu.sh",
    "date"=>"31 Oct 2007",
    "type"=>"sea",
);

$l51043ubuntu = array(
    "num"=>"5.10.43",
    "status"=>"Development version",
    "file"=>"boinc_ubuntu_5.10.43_i686-pc-linux-gnu.sh",
    "date"=>"26 Feb 2008",
    "type"=>"sea",
);

$l51028ubuntu = array(
    "num"=>"5.10.28",
    "status"=>"Recommended version",
    "file"=>"boinc_ubuntu_5.10.28_i686-pc-linux-gnu.sh",
    "date"=>"31 Oct 2007",
    "type"=>"sea",
);

$l51043ubuntux64 = array(
    "num"=>"5.10.43",
    "status"=>"Development version",
    "file"=>"boinc_ubuntu_5.10.43_x86_64-pc-linux-gnu.sh",
    "date"=>"26 Feb 2008",
    "type"=>"sea",
);

$l51028ubuntux64 = array(
    "num"=>"5.10.28",
    "status"=>"Recommended version",
    "file"=>"boinc_ubuntu_5.10.28_x86_64-pc-linux-gnu.sh",
    "date"=>"31 Oct 2007",
    "type"=>"sea",
);

$l5816 = array(
    "num"=>"5.8.16",
    "status"=>"Older version (includes GUI)",
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
    "description"=>"98/ME/2000/XP/Vista ",
    "dbname" => "windows_intelx86",
    "versions"=>array(
        $w51044,
        $w51030,
        $w5816,
    )
);

$windowsx64 = array(
    "name"=>"Windows 64-bit",
    "description"=>"XP/Vista",
    "dbname" => "windows_x86_64",
    "versions"=>array(
        $w51044x64,
        $w51030x64,
    )
);

$mac = array(
    "name"=>"Mac OS X",
    "description"=>"Version 10.3+",
    "dbname" => "powerpc-apple-darwin",
    "versions"=>array(
        $m51044,
        $m51044c,
        $m51034,
        $m51034c,
        $m5817,
        $m5817c,
    )
);

$linuxcompat = array(
    "name"=>"Older Linux x86 and x64",
    "description"=>"For older Linux versions, both x86 and x64; command-line interface only",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
        $l51043,
        $l51028,
        $l5816,
    )
);

$linux = array(
    "name"=>"Linux x86",
    "description"=>"For most current Linux/x86 versions; graphical and command-line interfaces",
    "dbname" => "i686-pc-linux-gnu",
    "versions"=>array(
        $l51043ubuntu,
        $l51028ubuntu,
    )
);

$linuxx64 = array(
    "name"=>"Linux x64",
    "description"=>"For most current Linux/x64 versions; graphical and command-line interfaces",
    "dbname" => "x86_64-pc-linux-gnu",
    "versions"=>array(
        $l51043ubuntux64,
        $l51028ubuntux64,
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
