<?php

ini_set('display_errors', 'stdout');
error_reporting(E_ALL);

// The platforms for a given project are stored in a file platforms/URL

function friendly_name($p) {
    $x = explode('[', $p);
    $pc = "";
    if (sizeof($x) > 1) {
        $p = $x[0];
        $pc = substr($x[1], 0, -1);
    }

    if (strstr($p, "fubar")) return null;
    if ($p == 'x86_64-unknown-linux-gnu') return null;
    $q = $p;
    switch ($p) {
    case 'i686-pc-linux-gnu': $q = 'Linux/x86'; break;
    case 'windows_intelx86': $q = 'Windows'; break;
    case 'x86_64-pc-linux-gnu': $q = 'Linux/x64'; break;
    case 'i686-apple-darwin': $q = 'Mac OS X'; break;
    case 'x86_64-apple-darwin': $q = 'Mac OS X 64-bit'; break;
    case 'powerpc-apple-darwin': $q = 'Mac OS X (PowerPC)'; break;
    case 'sparc-sun-solaris2.7': $q = 'SPARC Solaris 2.7'; break;
    case 'sparc-sun-solaris': $q = 'SPARC Solaris'; break;
    case 'powerpc64-unknown-linux-gnu': $q = 'Linux/PowerPC64'; break;
    case 'windows_x86_64': $q = 'Windows/x64'; break;
    case 'powerpc64-ps3-linux-gnu': $q = 'Playstation3/Linux'; break;
    case 'i386-portbld-freebsd': $q = 'FreeBSD/x86'; break;
    case 'windows_amd64': $q = 'Windows/Opteron'; break;
    case 'x86_64-pc-solaris': $q = 'Solaris/x64'; break;
    case 'windows_intelx86_64': $q = 'Windows/x64'; break;
    case 'arm-android-linux-gnu': $q = 'Android/ARM'; break;
    case 'arm-android': $q = 'Android/ARM'; break;
    case 'arm-unknown-linux-gnu': $q = 'Linux/ARM'; break;
    case 'x86_64-pc-freebsd': $q = 'FreeBSD/x86'; break;
    case 'arm-unknown-linux-gnueabihf': $q = 'Linux on ARM (e.g., Raspberry Pi)';
    }
    if (strlen($pc)) {
        if (strstr($pc, 'cuda')) $q .= " (NVIDIA GPU)";
        else if (strstr($pc, 'ati')) $q .= " (AMD/ATI GPU)";
        else if (strstr($pc, 'intel_gpu')) $q .= " (Intel GPU)";
        else if (strstr($pc, 'mt')) $q .= " (multicore)";
        else if (strstr($pc, 'vbox32')) $q .= " (VirtualBox 32-bit)";
        else if (strstr($pc, 'vbox64')) $q .= " (VirtualBox 64-bit)";
    }
    return $q;
}

function get_platform_icon_flags($l) {
    $x = new StdClass;
    $x->windows = false;
    $x->mac = false;
    $x->linux = false;
    $x->android = false;
    $x->freebsd = false;
    $x->rasp_pi = false;
    $x->nvidia_gpu = false;
    $x->amd_gpu = false;
    $x->intel_gpu = false;
    $x->vbox = false;
    foreach ($l as $p) {
        $y = explode('[', $p);
        $pc = "";
        if (sizeof($y) > 1) {
            $p = $y[0];
            $pc = substr($y[1], 0, -1);
        }
        if (strstr($p, "windows")) $x->windows = true;
        if (strstr($p, "darwin")) $x->mac = true;
        if (strstr($p, "linux")) $x->linux = true;
        if (strstr($p, "android")) $x->android = true;
        if (strstr($p, "freebsd")) $x->freebsd = true;
        if (strstr($p, "arm-unknown-linux-gnueabihf")) $x->rasp_pi = true;
        if (strstr($p, "armv5")) $x->rasp_pi = true;
        if (strstr($p, "armv6")) $x->rasp_pi = true;
        if (strstr($pc, "ati")) $x->amd_gpu = true;
        if (strstr($pc, "cuda")) $x->nvidia_gpu = true;
        if (strstr($pc, "intel_gpu")) $x->intel_gpu = true;
        if (strstr($pc, "vbox")) $x->vbox = true;
    }
    return $x;
}

function platform_icons($f) {
    $x = "";
    $ht = 40;
    $hs = 5;
    if ($f->windows) {
        $x .= "<img hspace=$hs height=$ht src=images/windows.jpg title=\"Supports Microsoft Windows\">";
    }
    if ($f->mac) {
        $x .= "<img hspace=$hs height=$ht src=images/mac.jpg title=\"Supports Mac OS X\">";
    }
    if ($f->linux) {
        $x .= "<img hspace=$hs height=$ht src=images/linux.jpg title=\"Supports Linux\">";
    }
    if ($f->android) {
        $x .= "<img hspace=$hs height=$ht src=images/android.png title=\"Supports Android\">";
    }
    if ($f->freebsd) {
        $x .= "<img hspace=$hs height=$ht src=images/freebsd.png title=\"Supports FreeBSD\">";
    }
    if ($f->rasp_pi) {
        $x .= "<img hspace=$hs height=$ht src=images/raspberry-pi.jpg title=\"Supports Raspberry Pi\">";
    }
    if ($f->nvidia_gpu) {
        $x .= "<img hspace=$hs height=$ht src=images/nvidia.jpg title=\"Supports NVIDIA GPUs\">";
    }
    if ($f->amd_gpu) {
        $x .= "<img hspace=$hs height=$ht src=images/amd_gpu.jpg title=\"Supports AMD GPUs\">";
    }
    if ($f->intel_gpu) {
        $x .= "<img hspace=$hs height=$ht src=images/intel_gpu.jpg title=\"Supports Intel GPUs\">";
    }
    if ($f->vbox) {
        $x .= "<img hspace=$hs height=$ht src=images/vbox.jpg title=\"Supports VirtualBox\">";
    }
    return $x;
}

function canonical_plan_class($pc) {
	if (strstr($pc, "atiOpenCL")) return "opencl_ati";
	if (strstr($pc, "nvidiaOpenCL")) return "opencl_nvidia";
	if (strstr($pc, "intelOpenCL")) return "opencl_intel_gpu";
    return $pc;
}

// get platforms from get_project_config.php (preferred method)
//
// format is either
//
// <project_config>
// <platforms>
//  <platform>windows_intelx86</platform>
//  ...
//
// or
//
// <project_config>
// <platforms>
//   <platform>
//    <platform_name>windows_intelx86</platform_name>
//    <user_friendly_name>Windows</user_friendly_name>
//    [<plan_class>xxx</plan_class>]
//   </platform>
//   ...
//
function get_platforms($url) {
    if (strstr($url, "radioactive")) return null;
    $url .= 'get_project_config.php';
    $x = @file_get_contents($url);
    if (!$x) {
        //echo "no file $url";
        return null;
    }
    libxml_use_internal_errors(true);
    $x = trim($x);
    $s = simplexml_load_string($x);
    if (!$s) {
        //echo "XML parse error";
        return null;
    }
    if (!array_key_exists('platforms', $s)) {
        return null;
    }
    $p = $s->platforms;
    //print_r($p);
    //echo "---\n";
    //foreach ($p->children() as $x) {
    //    echo $x;
    //}
    if (!array_key_exists('platform', $p)) {
        return null;
    }
    if (sizeof($p->platform) == 0) {
        return null;
    }
    $list = array();
    if (array_key_exists(0, $p->platform[0])) {
        foreach ($p->children() as $r) {
            $list[] = (string)$r;
        }
    } else {
        foreach ($p->platform as $r) {
            $plat = (string)$r->platform_name;
            if (array_key_exists('plan_class', $r)) {
                $pc = canonical_plan_class((string)$r->plan_class);
                if ($pc) {
                    $plat .= "[".$pc."]";
                }
            }
            $list[] = $plat;
        }
    }
    return array_unique($list);
}

// get platforms from app.php?xml=1 (deprecated)
//
function get_platforms2($url) {
    $url .= 'apps.php?xml=1';
    $x = @file_get_contents($url);
    if (!$x) {
        //echo "no file $url\n";
        return null;
    }
    libxml_use_internal_errors(true);
    $s = simplexml_load_string($x);
    $list = null;
    foreach($s->application as $a) {
        foreach ($a->version as $v) {
            if (!array_key_exists('platform_short', $v)) continue;
            $p = $v->platform_short[0];
            $pc = "";
            if (array_key_exists('plan_class', $v)) {
                $pc = (string)$v->plan_class;
                $pc = canonical_plan_class($pc);
            }
            if (strlen($pc)) {
                $p = $p.'['.$pc.']';
            }
            $list[] = (string)$p;
        }
    }
    return array_unique($list);
}

// convert an array of platform names into a comma-separated
// list of human-readable names
//
function make_friendly_string($l, $comma) {
    if (!count($l)) return "Unknown";
    $fn = array();
    foreach($l as $p) {
        $p = friendly_name($p);
        if (!$p) continue;
        $fn[] = $p;
    }
    $fn = array_unique($fn);
    $x = "";
    $first = true;
    foreach($fn as $p) {
        if ($first) {
            $x .= "$p";
            $first = false;
        } else {
            if ($comma) {
                $x .= ", $p";
            } else {
                $x .= "<br>$p";
            }
        }
    }
    return $x;
}

// return platforms as an array of platform names
//
function get_platforms_cached($url) {
    $u = urlencode($url);
    $fname = "/home/boincadm/boinc/doc/platforms/$u";
    $t = @filemtime($fname);
	if ($t && $t > time() - 604800) {
        $l = json_decode(file_get_contents($fname));
    } else {
        $l = get_platforms($url);
        if (!$l) {
            $l = get_platforms2($url);
        }
        if ($l) {
            file_put_contents($fname, json_encode($l));
        } else {
            if (file_exists($fname)) {
                touch($fname);
            } else {
                $l[] = "Unknown";
                file_put_contents($fname, json_encode($l));
            }
        }
    }
    return $l;
}

// return platforms as a human-readable string
//
function get_platforms_string($url, $comma=true) {
    $l = get_platforms_cached($url);
    return make_friendly_string($l, $comma);
}

function get_platform_icons($url) {
    $l = get_platforms_cached($url);
    $flags = get_platform_icon_flags($l);
    return platform_icons($flags);
}

//$u = "http://www.worldcommunitygrid.org/";
//$u = "http://asteroidsathome.net/boinc/";
//$u = "http://setiathome.berkeley.edu/";
//$u = "http://aqua.dwavesys.com/";
//$u = "http://lhcathome2.cern.ch/test4theory";
//$u = "http://www.rnaworld.de/rnaworld/";
//$u = "http://boinc.gorlaeus.net/";
//$u = "http://einstein.phys.uwm.edu/";
//$x = get_platforms($u);
//print_r($x);
//echo get_platforms_string($u);
//$x = "http://sat.isa.ru/pdsat/";
//print_r( get_platforms($x));


// Mediawiki extension to show a project's platforms.
//
function wfPlatforms() {
    global $wgParser;
    $wgParser->setHook( "platforms", "get_platforms_string" );
}

$wgExtensionFunctions[] = "wfPlatforms";

?>
