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

// PHP utility functions for cmdline tools and RPC handlers
// as well as web pages.
// Doesn't contain web-specific stuff like translation.inc

require_once("../inc/random_compat/random.inc");

// show PHP errors in output (e.g. web pages).
// Call this from your project.inc if you want.
// Not recommended for production projects;
// check the Apache error log instead.
//
function display_errors() {
    error_reporting(E_ALL);
    ini_set('display_errors', true);
    ini_set('display_startup_errors', true);
}

// always log errors
ini_set('log_errors', true);

// set to true in RPC handlers.
// Suppresses output that would invalidate the XML
$generating_xml = false;

// get project dir, assuming we're running in html/user or html/ops
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

function xml_error($num=-1, $msg=null, $file=null, $line=null) {
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

function get_config() {
    static $config;
    if ($config == null) {
        $d = project_dir();
        $config = file_get_contents("$d/config.xml");
    }
    return $config;
}

// Look for an element in a line of XML text
// If it's a single-tag element, and it's present, just return the tag
//
function parse_element($xml, $tag) {
    $closetag = "</" . substr($tag,1);
    $x = strstr($xml, $tag);
    if ($x) {
        if (strstr($tag, "/>")) return $tag;
        $y = substr($x, strlen($tag));
        $n = strpos($y, $closetag);
        if ($n) {
            $element = substr($y, 0, $n);
            return trim($element);
        }
    }
    return null;
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
    if (!$element) return null;
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

// uniform 0..1
//
function drand() {
    return ((double)rand())/getrandmax();
}

// does the plan class use a GPU?
//
function is_gpu($plan_class) {
    if (strstr($plan_class, "ati")) return true;
    if (strstr($plan_class, "cuda")) return true;
    if (strstr($plan_class, "nvidia")) return true;
    if (strstr($plan_class, "intel_gpu")) return true;
    if (strstr($plan_class, "apple_gpu")) return true;
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

// return hard-to-guess string of 32 random hex chars
//
function random_string() {
    return bin2hex(random_bytes(16));
}

// return high-resolution time
//
function dtime() {
    return microtime(true);
}

function date_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y', (int)$x);
}

function time_str($x) {
    if ($x == 0) return "---";
    return gmdate('j M Y, G:i:s', (int)$x) . " UTC";
}

function local_time_str($x) {
    if ($x == 0) return "---";
    return date('j M Y, H:i T', (int)$x);
}

function pretty_time_str($x) {
    return time_str($x);
}

// security vulnerabilities and user-supplied strings:
// sources:
// GET and POST arguments
//      including XML documents passed as args to RPC handlers
// cookies
//
// when used as SQL query args:
//      use BoincDb::escape_string() to prevent SQL injection
// when shown as HTML output
//      (e.g. 'not found' error pages, user names, forum posts)
//      use htmlspecialchars() to prevent XSS
// when used as file or dir name
//      use is_valid_filename()

// is $x a valid file (or dir) name?
// we want to avoid
//      FS traversal, e.g. "../../foo" or "/usr/lib/..."
//      shell command injection, e.g. "foo; rm*"
//      XSS stuff
// let's be conservative and allow only 'POSIX fully portable filenames',
// which can have only A-Z a-z 0-9 . - _
// In some cases filenames are used on volunteer hosts,
// whose OSs may have such restrictions.
// ALSO: allow spaces.
//
function is_valid_filename($x) {
    if (strlen($x)>255) return false;
    // \w means A-Za-z0-9_
    return preg_match('/^[\w\-. ]+$/', $x);
}

function filename_rules() {
    return 'Names can contain only A-Z a-z 0-9 . - _ space';
}

?>
