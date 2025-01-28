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

require_once("../inc/util_basic.inc");

// functions for doing arbitrary SQL queries, including joins.
// When possible, use the classes in boinc_db.inc instead.
// Lots of old code uses these functions, e.g. in ops/

define("MYSQLI", true);

if (MYSQLI) {
    function _mysql_connect($host, $user, $pass, $dbname) {
        global $mysqli;
        $x = explode(":", $host);
        if (sizeof($x)>1) {
            $host = $x[0];
            $port = $x[1];
        } else {
            $port = null;
        }
        $mysqli = @new mysqli($host, $user, $pass, $dbname, $port);
        if ($mysqli->connect_errno) {
            return null;
        }
        return $mysqli;
    }
    function _mysql_query($q) {
        global $mysqli;
        return mysqli_query($mysqli, $q);
    }
    function _mysql_num_rows($r) {
        return mysqli_num_rows($r);
    }
    function _mysql_num_fields($r) {
        global $mysqli;
        return mysqli_field_count($mysqli);
    }
    function _mysql_fetch_object($r) {
        return mysqli_fetch_object($r);
    }
    function _mysql_fetch_row($r) {
        return mysqli_fetch_row($r);
    }
    function _mysql_fetch_assoc($r) {
        return mysqli_fetch_assoc($r);
    }
    function _mysql_free_result($r) {
        return mysqli_free_result($r);
    }
    function _mysql_insert_id() {
        global $mysqli;
        return mysqli_insert_id($mysqli);
    }
    function _mysql_affected_rows() {
        global $mysqli;
        return mysqli_affected_rows($mysqli);
    }
    function _mysql_field_attrs($r, $i) {
        $x = mysqli_fetch_field_direct($r, $i);
        switch ($x->type) {
            case 1: $x->type = 'tinyint'; break;
            case 2: $x->type = 'smallint'; break;
            case 3: $x->type = 'int'; break;
            case 5: $x->type = 'double'; break;
            case 7: $x->type = 'timestamp'; break;
            case 252: $x->type = 'blob'; break;
            case 253: $x->type = 'varchar'; break;
            case 254: $x->type = 'char'; break;
        }
        return $x;
    }
    function _mysql_escape_string($x) {
        global $mysqli;
        return mysqli_escape_string($mysqli, $x);
    }
    function _mysql_error() {
        global $mysqli;
        return mysqli_error($mysqli);
    }
    function _mysql_fetch_array($r) {
        return mysqli_fetch_array($r);
    }
} else {
    function _mysql_connect($host, $user, $pass, $db_name) {
        $link = mysql_pconnect($host, $user, $pass);
        if (!$link) return null;
        if (!mysql_select_db($db_name, $link)) {
            return null;
        }
        return $link;
    }
    function _mysql_query($q) {
        return mysql_query($q);
    }
    function _mysql_num_rows($r) {
        return mysql_num_rows($r);
    }
    function _mysql_num_fields($r) {
        return mysql_num_fields($r);
    }
    function _mysql_fetch_object($r) {
        return mysql_fetch_object($r);
    }
    function _mysql_fetch_row($r) {
        return mysql_fetch_row($r);
    }
    function _mysql_fetch_assoc($r) {
        return mysql_fetch_assoc($r);
    }
    function _mysql_free_result($r) {
        return mysql_free_result($r);
    }
    function _mysql_insert_id() {
        return mysql_insert_id();
    }
    function _mysql_affected_rows() {
        return mysql_affected_rows();
    }
    function _mysql_field_attrs($r, $i) {
        $x = new StdClass;
        $x->name = mysql_field_name($r, $i);
        $x->type = mysql_field_type($r, $i);
        $x->length = mysql_field_len($r, $i);
        return $x;
    }
    function _mysql_escape_string($x) {
        return mysql_escape_string($x);
    }
    function _mysql_error() {
        return mysql_error();
    }
    function _mysql_fetch_array($r) {
        return mysql_fetch_array($r);
    }
}

function db_init_aux($try_replica=false) {
    $config = get_config();
    $user = parse_config($config, "<db_user>");
    $pass = parse_config($config, "<db_passwd>");
    $db_name = parse_config($config, "<db_name>");
    $host = null;
    if ($try_replica) {
        $x = parse_config($config, "<replica_db_host>");
        if ($x) {
            $host = $x;
            $x = parse_config($config, "<replica_db_user>");
            if ($x) $user = $x;
            $x = parse_config($config, "<replica_db_passwd>");
            if ($x) $pass = $x;
            $x = parse_config($config, "<replica_db_name>");
            if ($x) $db_name = $x;
        }
    }
    if ($host == null) {
        $host = parse_config($config, "<db_host>");
    }
    if ($host == null) {
        $host = "localhost";
    }
    if (1) {
        if (!_mysql_connect($host, $user, $pass, $db_name)) {
            return 1;
        }
    } else {
        $link = mysql_pconnect($host, $user, $pass);
        if (!$link) {
            return 1;
        }
        if (!mysql_select_db($db_name, $link)) {
            echo "selecting $db_name\n";
            return 2;
        }
    }

    return 0;
}

// escape a string for MySQL "like"
//
function escape_pattern($str) {
    $str = str_replace('_', '\\\\_', $str);
    $str = str_replace('%', '\\\\%', $str);
    return $str;
}

?>
