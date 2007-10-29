<?php

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
    return BoincUser::lookup("authenticator='$auth'");
}

function lookup_user_id($id) {
    return BoincUser::lookup_id($id);
}

function lookup_user_email_addr($email_addr) {
    return BoincUser::lookup("email_addr='$email_addr'");
}

function lookup_user_name($name) {
    $teams = BoincTeam::enum("name='".boinc_real_escape_string($name)."'");
    if (sizeof($teams)==1) {
        return $teams[0];
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
    return BoincTeam::lookup("name='$name'");
}

function lookup_wu($id) {
    $result = mysql_query("select * from workunit where id=$id");
    if ($result) {
        $wu = mysql_fetch_object($result);
        mysql_free_result($result);
        return $wu;
    }
    return null;
}

function lookup_result($id) {
    $result = mysql_query("select * from result where id=$id");
    if ($result) {
        $r = mysql_fetch_object($result);
        mysql_free_result($result);
        return $r;
    }
    return null;
}

function lookup_app($id) {
    $result = mysql_query("select * from app where id=$id");
    if ($result) {
        $app = mysql_fetch_object($result);
        mysql_free_result($result);
        return $app;
    }
    return null;
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

// Process user-supplied text prior to using in query;
// trims whitespace and escapes quotes.
// Does NOT remove HTML tags.
//
function process_user_text($value) {
    $value = trim($value);
    if (get_magic_quotes_gpc()) {
        $value = stripslashes($value);
    }
    return boinc_real_escape_string($value);
}

// escape a string for MySQL "like"
//
function escape_pattern($str) {
    $str = str_replace('_', '\\\\_', $str);
    $str = str_replace('%', '\\\\%', $str);
    return $str;
}

?>
