<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// minimal set of utility functions, usable outside a BOINC project.
// Doesn't pull in translation.inc etc.

$generating_xml = false;

function project_dir() {
    $d = dirname(__FILE__);
    return "$d/../..";
}

function web_stopped() {
    $d = project_dir();
    return file_exists("$d/stop_web");
}

function sched_stopped() {
    $d = project_dir();
    return file_exists("$d/stop_sched");
}

function show_page($x, $y) {
    echo "
        <title>$x</title>
        <h1>$x</h1>
        $y
    ";
}

function xml_error($num, $msg=null, $file=null, $line=null) {
    global $xml_outer_tag;
    if (!$msg) {
        switch($num) {
        case -112: $msg = "Invalid XML"; break;
        case -136: $msg = "Not found"; break;
        case -137: $msg = "Name or email address is not unique"; break;
        case -138: $msg = "Can't access database"; break;
        case -183: $msg = "Project is temporarily offline"; break;
        case -205: $msg = "Email address has invalid syntax"; break;
        case -206: $msg = "Invalid password"; break;
        case -207: $msg = "Email address is not unique"; break;
        case -208: $msg = "Account creation is disabled"; break;
        case -209: $msg = "Invalid invitation code"; break;
        case -210: $msg = "Invalid request method"; break;
        default: $msg = "Unknown error"; break;
        }
    }
    echo "<error>
    <error_num>$num</error_num>
    <error_msg>$msg</error_msg>
";
    if ($file) {
        echo "    <file>$file</file>\n";
    }
    if ($line) {
        echo "    <line>$line</line>\n";
    }
    echo "</error>\n";
    if (isset($xml_outer_tag) && $xml_outer_tag != "") {
        echo "</$xml_outer_tag>\n";
    }
    exit();
}

$g_config = null;
function get_config() {
    global $g_config;
    if ($g_config == null) {
        $d = project_dir();
        $g_config = file_get_contents("$d/config.xml");
    }
    return $g_config;
}

// Look for an element in a line of XML text
// If it's a single-tag element, and it's present, just return the tag
//
function parse_element($xml, $tag) {
    $element = null;
    $closetag = "</" . substr($tag,1);
    $x = strstr($xml, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, $closetag);
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return trim($element);
}

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

// return true if XML contains either <tag/> or <tag>1</tag>
//
function parse_bool($xml, $tag) {
    $x = "<$tag/>";
    if (strstr($xml, $x)) return true;
    $x = "<$tag>";
    $y = (int)parse_element($xml, $x);
    if ($y != 0) return true;
    return false;
}

// look for a particular element in the config file
//
function parse_config($config, $tag) {
    $element = parse_element($config, $tag);
    return $element;
}

function drand() {
    return ((double)rand())/getrandmax();
}

// kludge
//
function is_gpu($plan_class) {
    if (strstr($plan_class, "ati")) return true;
    if (strstr($plan_class, "cuda")) return true;
    if (strstr($plan_class, "nvidia")) return true;
    if (strstr($plan_class, "intel_gpu")) return true;
    return false;
}

// the same as file_get_contents() but uses curl
//
function url_get_contents($url) {
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_HEADER, false);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_MAXREDIRS, 5);
    $content = curl_exec($ch);
    curl_close($ch);
    return $content;
}

// Stuff related to password hashing.
// Originally we hashed with MD5.
// But MD5 is so fast that brute-force cracking would be easy
// for hackers who break into a project's server.
// So now we additionally hash with bcrypt,
// if available (PHP >= 5.5) via PHP's password_hash() function.
//
// So there are two levels of hash:
//
// hash_0: md5(password.email_addr)
// hash_1: password_hash(hash_0, PASSWORD_DEFAULT)
//
// hash_0 is what gets sent over the network,
// from the client and account manager RPCs.
//
// hash_1 is what gets stored in the DB
//   except that hash_0 is stored in the DB if
//   - the project has pre-5.5 PHP, or
//   - the project hasn't run the update script (see below)
//
// It would be slightly more secure if hash_1 was used on the network.
// But that would require a client and account manager change,
// and it would require them to know whether the server has password_hash().
//
// We supply a script update_password_hash.php.
// This changes the user table from hash_0 to hash_1.
// Projects can run it whenever they want;
// those using old PHP can wait until they upgrade PHP.
//
// To simplify the transition:
// - When we're authorizing a user, 

?>
