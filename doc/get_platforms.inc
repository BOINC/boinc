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
    case 'x86_64-pc-linux-gnu': return 'Linux/x86_64';
    case 'i686-apple-darwin': return 'Mac OS X';
    case 'powerpc-apple-darwin': return 'Mac OS X (PowerPC)';
    case 'sparc-sun-solaris2.7': return 'SPARC Solaris 2.7';
    case 'sparc-sun-solaris': return 'SPARC Solaris';
    case 'powerpc64-unknown-linux-gnu': return 'Linux/PowerPC64';
    case 'windows_x86_64': return 'Windows/x86_64';
    case 'powerpc64-ps3-linux-gnu': return 'Playstation3/Linux';
    case 'x86_64-unknown-linux-gnu': return null;
    }
    if (strstr($p, "fubar")) return null;
    return $p;
}

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
        $list[] = friendly_name($p);
    }
    return array_unique($list);
}

function show_list($l) {
    $x = "";
    $first = true;
    foreach($l as $p) {
        if ($first) {
            $x .= "$p";
            $first = false;
        } else {
            $x .= ", $p";
        }
    }
    return $x;
}

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
            if (!file_exists($fname)) {
                $l[] = "Unknown";
                file_put_contents($fname, json_encode($l));
            }
        }
    }
    return show_list($l);
}
//echo get_platforms_cached("http://www.malariacontrol.net/");

function wfPlatforms() {
    global $wgParser;
    $wgParser->setHook( "platforms", "get_platforms_cached" );
}

$wgExtensionFunctions[] = "wfPlatforms";

?>
