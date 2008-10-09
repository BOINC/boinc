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

// minimal set of util functions;
// doesn't pull in translation.inc etc.

function web_stopped() {
    return file_exists("../../stop_web");
}

function sched_stopped() {
    return file_exists("../../stop_sched");
}

function show_page($x, $y) {
    echo "
        <title>$x</title>
        <h1>$x</h1>
        $y
    ";
}

function xml_error($num, $msg=null) {
    if (!$msg) {
        switch($num) {
        case -112: $msg = "Invalid XML"; break;
        case -136: $msg = "Invalid ID"; break;
        case -137: $msg = "Name or email address is not unique"; break;
        case -138: $msg = "Can't access database"; break;
        case -183: $msg = "Project is temporarily offline"; break;
        case -205: $msg = "Email address has invalid syntax"; break;
        case -206: $msg = "Invalid password"; break;
        case -207: $msg = "Email address is not unique"; break;
        case -208: $msg = "Account creation is disabled"; break;
        case -209: $msg = "Invalid invitation code"; break;
	case -210: $msg = "Invalid request method"; break;
        default: "Unknown error"; break;
        }
    }
    echo "<error>
    <error_num>$num</error_num>
    <error_msg>$msg</error_msg>
</error>
";
    exit();
}

$g_config = null;
function get_config() {
    global $g_config;
    if ($g_config == null) {
       $g_config = file_get_contents("../../config.xml");
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

// look for a particular element in the ../../config.xml file
//
function parse_config($config, $tag) {
    $element = parse_element($config, $tag);
    return $element;
}

function drand() {
    return ((double)rand())/getrandmax();
}

?>
