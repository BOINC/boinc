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

require_once('../inc/util.inc');
require_once('../inc/boinc_db.inc');

// database-related functions.
// Presentation code (HTML) shouldn't be here

function db_init_aux($try_replica=false) {
    $config = get_config();
	$user = parse_config($config, "<db_user>");
	$pass = parse_config($config, "<db_passwd>");
    $host = null;
    if ($try_replica == true) {
	  $host = parse_config($config, "<replica_db_host>");
    }
    if ($host == null) {
  	  $host = parse_config($config, "<db_host>");
    }
    if ($host == null) {
        $host = "localhost";
    }
    $link = mysql_pconnect($host, $user, $pass);
    if (!$link) {
        return 1;
    }
    $db_name = parse_config($config, "<db_name>");
    if (!mysql_select_db($db_name, $link)) {
        echo "selecting $db_name\n";
        return 2;
    }

    return 0;
}

function lookup_user_auth($auth) {
    $auth = BoincDb::escape_string($auth);
    return BoincUser::lookup("authenticator='$auth'");
}

function lookup_user_id($id) {
    return BoincUser::lookup_id($id);
}

function lookup_user_email_addr($email_addr) {
    $e = BoincDb::escape_string($email_addr);
    return BoincUser::lookup("email_addr='$e'");
}

function lookup_user_name($name) {
    $name = BoincDb::escape_string($name);
    $users = BoincUser::enum("name='".boinc_real_escape_string($name)."'");
    if (sizeof($users)==1) {
        return $users[0];
    }
    return null;
}

function lookup_host($id) {
    return BoincHost::lookup_id($id);
}

function lookup_team($id) {
    return BoincTeam::lookup_id($id);
}

function lookup_team_founder($id) {
    return BoincTeam::lookup("userid=$id");
}

function lookup_team_name($name) {
    $name = BoincDb::escape_string($name);
    return BoincTeam::lookup("name='$name'");
}

function lookup_wu($id) {
    return BoincWorkunit::lookup_id($id);
}

function lookup_result($id) {
    return BoincResult::lookup_id($id);
}

function lookup_app($id) {
    return BoincApp::lookup_id($id);
}

// apply this to any user-supplied strings used in queries
//
function boinc_real_escape_string($x) {
    if (version_compare(phpversion(),"4.3.0")>=0) {
        return BoincDb::escape_string($x);
    } else {
        $x = str_replace("'", "\'", $x);
        $x = str_replace("\"", "\\\"", $x);
        return $x;
    }
}

// escape a string for MySQL "like"
//
function escape_pattern($str) {
    $str = str_replace('_', '\\\\_', $str);
    $str = str_replace('%', '\\\\%', $str);
    return $str;
}

?>
