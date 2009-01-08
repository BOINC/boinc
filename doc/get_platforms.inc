<?php

// Mediawiki extension to show a project's platforms.
// The platforms for a given project are stored in a file platforms/URL
//
function parse_next_element($xml, $tag, &$cursor) {
    $element = null;
    $closetag = "</" . substr($tag,1);
    $pos = substr($xml,$cursor);
    $x = strstr($pos, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, $closetag);
        if ($n) {
            $element = substr($y, 0, $n);
        }
        $cursor = (strlen($xml) - strlen($x)) + strlen($tag) + strlen($closetag) + strlen($element);
    }
    return trim($element);
}

function friendly_name($p) {
    switch ($p) {
    case 'i686-pc-linux-gnu': return 'Linux/x86';
    case 'windows_intelx86': return 'Windows';
    case 'x86_64-pc-linux-gnu': return 'Linux/x64';
    case 'i686-apple-darwin': return 'Mac OS X';
    case 'x86_64-apple-darwin': return 'Mac OS X 64-bit';
    case 'powerpc-apple-darwin': return 'Mac OS X (PowerPC)';
    case 'sparc-sun-solaris2.7': return 'SPARC Solaris 2.7';
    case 'sparc-sun-solaris': return 'SPARC Solaris';
    case 'powerpc64-unknown-linux-gnu': return 'Linux/PowerPC64';
    case 'windows_x86_64': return 'Windows/x64';
    case 'powerpc64-ps3-linux-gnu': return 'Playstation3/Linux';
    case 'x86_64-unknown-linux-gnu': return null;
    }
    if (strstr($p, "fubar")) return null;
    return $p;
}

// 
function get_platforms($url) {
    $url .= '/get_project_config.php';
    $x = file_get_contents($url);
    if (!$x) return null;
    $cursor = 0;
    $list = null;
    $tag = '<platform_name>';
    if (!strstr($x, "<platform_name>")) $tag = '<platform>';
    while (1) {
        $p = parse_next_element($x, $tag, $cursor);
        if (!$p) break;
        $list[] = $p;
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

//echo get_platforms_string("http://www.malariacontrol.net/");

function wfPlatforms() {
    global $wgParser;
    $wgParser->setHook( "platforms", "get_platforms_string" );
}

$wgExtensionFunctions[] = "wfPlatforms";

?>
