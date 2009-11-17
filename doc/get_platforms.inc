<?php

ini_set('display_errors', 'stdout');
error_reporting(E_ALL);

// Mediawiki extension to show a project's platforms.
// The platforms for a given project are stored in a file platforms/URL
//

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
    }
    if (strlen($pc)) {
        if (strstr($pc, 'cuda')) $q .= " (NVIDIA GPU)";
        else if (strstr($pc, 'ati')) $q .= " (ATI GPU)";
        else if (strstr($pc, 'mt')) $q .= " (multicore)";
    }
    return $q;
}

// get platforms from get_project_config.php (preferred method)
//
function get_platforms(&$url) {
    $url .= 'get_project_config.php';
    $x = file_get_contents($url);
    if (!$x) return null;
    $s = new SimpleXMLElement($x);
    if (array_key_exists('rpc_prefix', $s)) {
        $url = $s->rpc_prefix;
    }
    if (!array_key_exists('platforms', $s)) return null;
    $p = $s->platforms;
    if (!array_key_exists('platform', $p)) return null;
    if (sizeof($p->platform) == 0) return null;
    if (array_key_exists(0, $p->platform[0])) {
        foreach ($p->platform as $r) {
            $list[] = (string)$r->platform_name;
        }
    } else {
        $list = array();
        foreach ($p->platform as $r) {
            print_r($r);
            if (array_key_exists('plan_class', $r)) {
                $list[] = (string)$r->platform_name.'['.(string)$r->plan_class.']';
            } else {
                $list[] = (string)$r->platform_name;
            }
        }
    }
    return array_unique($list);
}

// get platforms from app.php?xml=1 (deprecated)
//
function get_platforms2($url) {
    $url .= 'apps.php?xml=1';
    $x = file_get_contents($url);
    if (!$x) return null;
    $s = new SimpleXMLElement($x);
    $list = null;
    foreach($s->application as $a) {
        foreach ($a->version as $v) {
            if (!array_key_exists('platform_short', $v)) continue;
            $p = $v->platform_short[0];
            $pc = "";
            if (array_key_exists('plan_class', $v)) {
                $pc = $v->plan_class[0];
            }
            if (strlen($pc)) {
                $list[] = $p.'['.$pc.']';
            } else {
                $list[] = $p;
            }
        }
    }
    return array_unique($list);
}

// convert an array of platform names into a comma-separated
// list of human-readable names
//
function make_friendly_string($l) {
    if (!count($l)) return "Unknown";
    $x = "";
    $first = true;
    foreach($l as $p) {
        $p = friendly_name($p);
        if (!$p) continue;
        if ($first) {
            $x .= "$p";
            $first = false;
        } else {
            $x .= ", $p";
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
    if ($t && $t > time() - 86400) {
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
function get_platforms_string($url) {
    $l = get_platforms_cached($url);
    return make_friendly_string($l);
}

//echo get_platforms_string("http://www.worldcommunitygrid.org/");
//echo get_platforms_string("http://setiathome.berkeley.edu/");
//echo get_platforms_string("http://dist.ist.tugraz.at/cape5/");
//print_r(get_platforms2("http://www.primegrid.com/"));

function wfPlatforms() {
    global $wgParser;
    $wgParser->setHook( "platforms", "get_platforms_string" );
}

$wgExtensionFunctions[] = "wfPlatforms";

?>
