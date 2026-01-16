<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// A project can have one or more BOINC databases:
// DB 0:
//      the main DB; read/write
//      identified in config file by db_host, db_name, db_user, db_passwd
//      db_host defaults to localhost
// DB 1:
//      read-only replica; identified by
//      replica_db_host/name/user/passwd (must include all)
// DB 2:
//      read-only replica; identified by
//      replica2_db_host/name/user/passwd (must include all)
// ... and potentially more

function incs() {
    $d = dirname(__FILE__);
    require_once("$d/db_conn.inc");
    require_once("$d/util_basic.inc");
}

incs();

// class BoincDb represents a connection to a BOINC database.
// All its members are static, so there's only 1 connection at a time.
// get(n) establishes a connection to DB n,
// or DB 0 if that fails or doesn't exit.
// close() closes the connection.

class BoincDb {
    static $instance;       // a DbConn object, or null
    static $dbnum;          // which replica we're connected to

    // connect to DB $dbnum (0, 1, ...)
    // If the requested DB doesn't exist or connection fails, connect to DB 0.
    // Set self::$instance; no return value
    //
    static function get_aux($dbnum) {
        $instance = new DbConn();
        self::$instance = null;
        $config = get_config();
        if ($dbnum) {
            $r = $dbnum==1?'':strval($dbnum);
            $host = parse_config($config, sprintf('<replica%s_db_host>', $r));
            $name = parse_config($config, sprintf('<replica%s_db_name>', $r));
            $user = parse_config($config, sprintf('<replica%s_db_user>', $r));
            $passwd = parse_config($config, sprintf('<replica%s_db_passwd>', $r));
            if ($host && $name && $user && $passwd) {
                $retval = $instance->init_conn($user, $passwd, $host, $name);
                if ($retval) {
                    //error_log("BoincDb::get_aux(): connected to replica DB $dbnum");
                    self::$instance = $instance;
                    self::$dbnum = $dbnum;
                    return;
                }
            }
            // if can't connect to replica, fall through and try DB 0
        }
        $host = parse_config($config, '<db_host>');
        if (!$host) $host = 'localhost';
        $user = parse_config($config, '<db_user>');
        $name = parse_config($config, '<db_name>');
        $passwd = parse_config($config, '<db_passwd>');
        if (!$name || !$user || !$passwd) {
            error_log("BoincDb::get_aux(): must specify DB name, user, passwd");
            return;
        }
        $retval = $instance->init_conn($user, $passwd, $host, $name);
        if ($retval) {
            //error_log("BoincDb::get_aux(): connected to DB $dbnum");
            self::$instance = $instance;
            self::$dbnum = 0;
            return;
        }
        error_log("BoincDb::get_aux(): Couldn't connect to DB $dbnum");
    }

    // connect to DB $dbnum, but first:
    // 1) check for a cached connection
    // 2) check whether the "stop_web" trigger file is present
    //
    // If there's a page that's guaranteed to do only reads, put
    // BoincDb::get(true);
    // at the top of it.
    //
    // Note: true == 1.
    // You can also use 2, 3... to select other replicas
    //
    static function get($dbnum = 0) {
        global $generating_xml;
        if (isset(self::$instance)) {
            if (intval(self::$dbnum) == intval($dbnum)) {
                return self::$instance;
            }
            self::close();
        }
        if (web_stopped()) {
            if ($generating_xml) {
                xml_error(-183, "project down for maintenance");
            } else {
                show_project_down();
            }
        }
        self::get_aux($dbnum);
        if (self::$instance) {
            return self::$instance;
        }
        if ($generating_xml) {
            xml_error(-138, "Can't connect to database");
        } else {
            error_page("Can't connect to database");
        }
    }

    static function close() {
        if (isset(self::$instance)) {
            self::$instance->close();
            self::$instance = null;
        }
    }

    static function escape_string($string) {
        if (!$string) return '';
        $db = self::get();
        return $db->base_escape_string(trim($string));
    }
    static function error() {
        $db = self::get();
        return $db->base_error();
    }

    // if your DB connection times out, call this, then call get() again
    //
    static function reset_connection() {
        self::$instance = null;
    }
}

#[AllowDynamicProperties]
class BoincUser {
    public $prefs;
    static $cache;
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('user', 'BoincUser', $clause);
    }

    static function lookup_id_nocache($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'user', 'BoincUser');
    }
    static function lookup_id($id) {
        if (!isset(self::$cache[$id])) {
            self::$cache[$id] = self::lookup_id_nocache($id);
        }
        return self::$cache[$id];
    }
    static function lookup_auth($auth) {
        $auth = BoincDb::escape_string($auth);
        return self::lookup("authenticator='$auth'");
    }
    static function lookup_email_addr($email_addr) {
        $email_addr = strtolower(BoincDb::escape_string($email_addr));
        return self::lookup("email_addr='$email_addr'");
    }
    //This is to find email addresses that have changed in the past 7 days.
    static function lookup_prev_email_addr($email_addr) {
        $email_addr = strtolower(BoincDb::escape_string($email_addr));
        $mytime = time() - 604800;
        return self::lookup("email_addr_change_time > $mytime and previous_email_addr='$email_addr'");
    }
    // name is not necessarily unique
    //
    static function lookup_name($name) {
        $name = BoincDb::escape_string($name);
        $users = BoincUser::enum("name='$name'");
        return $users;
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('user', $clause);
    }
    static function max($field) {
        $db = BoincDb::get();
        return $db->max('user', $field);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'user', $clause);
    }
    static function enum($where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('user', 'BoincUser', $where_clause, $order_clause);
    }
    static function enum_fields($fields, $where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum_fields(
            'user', 'BoincUser', $fields, $where_clause, $order_clause
        );
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('user', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }
    function delete() {
        $db = BoincDb::get();
        $db->delete_aux('profile', "userid=$this->id");
        return $db->delete($this, 'user');
    }
    static function sum($field) {
        $db = BoincDb::get();
        return $db->sum('user', $field);
    }
    static function percentile($field, $clause, $pct) {
        $db = BoincDb::get();
        return $db->percentile('user', $field, $clause, $pct);
    }
}

#[AllowDynamicProperties]
class BoincTeam {
    static $cache;
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('team', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }
    static function lookup_id_nocache($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'team', 'BoincTeam');
    }
    static function lookup_id($id) {
        if (!isset(self::$cache[$id])) {
            self::$cache[$id] = self::lookup_id_nocache($id);
        }
        return self::$cache[$id];
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'team', $clause);
    }
    static function enum($where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('team', 'BoincTeam', $where_clause, $order_clause);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('team', 'BoincTeam', $clause);
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        $name = BoincDb::escape_string($name);
        return self::lookup("name='$name'");
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'team');
    }
    static function percentile($field, $clause, $pct) {
        $db = BoincDb::get();
        return $db->percentile('team', $field, $clause, $pct);
    }
    static function max($field) {
        $db = BoincDb::get();
        return $db->max('team', $field);
    }
    static function enum_fields($fields, $where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum_fields(
            'team', 'BoincTeam', $fields, $where_clause, $order_clause
        );
    }
}

#[AllowDynamicProperties]
class BoincTeamDelta {
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('team_delta', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('team_delta', 'BoincTeamDelta', $where_clause);
    }
    static function delete_for_user($user_id) {
        $db = BoincDb::get();
        return $db->delete_aux('team_delta', "userid=$user_id");
    }
}

#[AllowDynamicProperties]
class BoincHost {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'host', 'BoincHost');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'host', $clause);
    }
    static function update_aux($clause, $where='') {
        $db = BoincDb::get();
        return $db->update_aux('host', $clause, $where);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'host');
    }
    static function enum($where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('host', 'BoincHost', $where_clause, $order_clause);
    }
    static function enum_fields($fields, $where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum_fields(
            'host', 'BoincHost', $fields, $where_clause, $order_clause
        );
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('host', $clause);
    }
    static function lookup_cpid($cpid) {
        $db = BoincDb::get();
        $cpid = BoincDb::escape_string($cpid);
        return $db->lookup('host', 'BoincHost', "host_cpid='$cpid'");
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('host', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    static function delete_for_user($user_id) {
        $db = BoincDb::get();
        return $db->delete_aux('host', "userid=$user_id");
    }
}

#[AllowDynamicProperties]
class BoincResult {
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('result', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('result', 'BoincResult', $where_clause);
    }
	static function enum_fields($fields, $where_clause, $order_clause=null) {
        $db = BoincDb::get();
		return $db->enum_fields(
            'result', 'BoincResult', $fields, $where_clause, $order_clause
        );
	}
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'result', $clause);
    }
    static function update_aux($clause, $where='') {
        $db = BoincDb::get();
        return $db->update_aux('result', $clause, $where);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'result', 'BoincResult');
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        $name = BoincDb::escape_string($name);
        return $db->lookup('result', 'BoincResult', "name='$name'");
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'result');
    }
}

#[AllowDynamicProperties]
class BoincWorkunit {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'workunit', 'BoincWorkunit');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('workunit', 'BoincWorkunit', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('workunit', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('workunit', 'BoincWorkunit', $where_clause);
    }
    static function enum_fields($fields, $where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum_fields('workunit', 'BoincWorkunit', $fields, $where_clause, $order_clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'workunit', $clause);
    }
    static function update_aux($clause, $where='') {
        $db = BoincDb::get();
        return $db->update_aux('workunit', $clause, $where);
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('workunit', $clause);
    }
}

#[AllowDynamicProperties]
class BoincApp {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'app', 'BoincApp');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('app', 'BoincApp', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('app', 'BoincApp', $where_clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('app', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'app', $clause);
    }
    static function sum($field, $clause=null) {
        $db = BoincDb::get();
        return $db->sum('app', $field, $clause);
    }
}

#[AllowDynamicProperties]
class BoincAppVersion {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('app_version', 'BoincAppVersion', $where_clause);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('app_version', 'BoincAppVersion', $clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'app_version', 'BoincAppVersion');
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('app_version', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'app_version', $clause);
    }
}

#[AllowDynamicProperties]
class BoincProfile {
    static function lookup_fields($fields, $clause) {
        $db = BoincDb::get();
        return $db->lookup_fields('profile', 'BoincProfile', $fields, $clause);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('profile', 'BoincProfile', $clause);
    }
    static function lookup_userid($userid) {
        $db = BoincDb::get();
        return $db->lookup('profile', 'BoincProfile', 'userid='.$userid);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('profile', $clause.' where userid='.$this->userid);
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('profile', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('profile', $clause);
    }
    static function enum($where_clause=null, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('profile', 'BoincProfile', $where_clause, $order_clause);
    }
    static function enum_fields($fields, $where_clause=null, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum_fields('profile', 'BoincProfile', $fields, $where_clause, $order_clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete_aux('profile', 'userid='.$this->userid);
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('profile', $clause);
    }
}

#[AllowDynamicProperties]
class BoincTeamAdmin {
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('team_admin', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('team_admin', 'BoincTeamAdmin', $where_clause);
    }
    static function delete($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('team_admin', $clause);
    }
    static function lookup($teamid, $userid) {
        $db = BoincDb::get();
        return $db->lookup('team_admin', 'BoincTeamAdmin', "teamid=$teamid and userid=$userid");
    }
}

#[AllowDynamicProperties]
class BoincPrivateMessage {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'private_messages', 'BoincPrivateMessage');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'private_messages', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('private_messages', 'BoincPrivateMessage', $where_clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('private_messages', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('private_messages', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'private_messages');
    }
    static function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('private_messages', $clause);
    }
}

#[AllowDynamicProperties]
class BoincPlatform {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('platform', 'BoincPlatform', $where_clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'platform', 'BoincPlatform');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('platform', 'BoincPlatform', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'platform', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('platform', $clause);
    }
}

#[AllowDynamicProperties]
class BoincHostAppVersion {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('host_app_version', 'BoincHostAppVersion', $where_clause);
    }
    static function lookup($host_id, $app_version_id) {
        $db = BoincDb::get();
        return $db->lookup(
            'host_app_version', 'BoincHostAppVersion',
            "host_id=$host_id and app_version_id=$app_version_id"
        );
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('host_app_version', $clause);
    }
    static function delete_for_user($user_id) {
        $db = BoincDb::get();
        return $db->delete_aux('host_app_version', "host_id in (select id from host where userid = $user_id)");
    }
}

// DB utility functions

// return the "latest" app versions for a given app and platform
//
function latest_avs_app_platform($appid, $platformid) {
    $avs = BoincAppVersion::enum(
        "appid=$appid and platformid = $platformid and deprecated=0"
    );
    foreach ($avs as $av) {
        foreach ($avs as $av2) {
            if ($av->id == $av2->id) continue;
            if ($av->plan_class == $av2->plan_class && $av->beta == $av2->beta && $av->version_num > $av2->version_num) {
                $av2->deprecated = 1;
            }
        }
    }
    $r = array();
    foreach ($avs as $av) {
        if (!$av->deprecated) {
            $r[] = $av;
        }
    }
    return $r;
}

// return the "latest" app versions for a given app
//
function latest_avs_app($appid) {
    $platforms = BoincPlatform::enum("");
    $r = array();
    foreach ($platforms as $p) {
        $avs = latest_avs_app_platform($appid, $p->id);
        $r = array_merge($r, $avs);
    }
    return $r;
}

#[AllowDynamicProperties]
class BoincBadge {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('badge', 'BoincBadge', $where_clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('badge', $clause);
        if (!$ret) return 0;
        return $db->insert_id();
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'badge', $clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'badge', 'BoincBadge');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('badge', 'BoincBadge', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'badge');
    }
}

#[AllowDynamicProperties]
class BoincBadgeUser {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('badge_user', 'BoincBadgeUser', $where_clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('badge_user', $clause);
        if (!$ret) return false;
        return true;
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('badge_user', 'BoincBadgeUser', $clause);
    }
    static function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('badge_user', $clause);
    }
    static function delete($clause) {
        $db = BoincDb::get();
        $db->delete_aux('badge_user', $clause);
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('badge_user', $clause);
    }
}

#[AllowDynamicProperties]
class BoincBadgeTeam {
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('badge_team', 'BoincBadgeTeam', $where_clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('badge_team', $clause);
        if (!$ret) return false;
        return true;
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('badge_team', 'BoincBadgeTeam', $clause);
    }
    static function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('badge_team', $clause);
    }
    static function delete($clause) {
        $db = BoincDb::get();
        $db->delete_aux('badge_team', $clause);
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('badge_team', $clause);
    }
}

#[AllowDynamicProperties]
class BoincCreditUser {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('credit_user', 'BoincCreditUser', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('credit_user', 'BoincCreditUser', $where_clause);
    }
    static function sum($field, $clause) {
        $db = BoincDb::get();
        return $db->sum('credit_user', $field, $clause);
    }
    static function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('credit_user', $clause);
    }
    static function delete_user($user) {
        $db = BoincDb::get();
        $db->delete_aux('credit_user', "userid=$user->id");
    }
    static function get_list($where_clause, $order_clause, $limit) {
        $db = BoincDB::get();
        return $db->get_list('user', 'credit_user', 'id', 'userid', 'BoincCreditUser', '*', $where_clause, $order_clause, $limit);
    }
}

#[AllowDynamicProperties]
class BoincCreditTeam {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('credit_team', 'BoincCreditTeam', $clause);
    }
    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('credit_team', 'BoincCreditTeam', $where_clause);
    }
    static function sum($field, $clause) {
        $db = BoincDb::get();
        return $db->sum('credit_team', $field, $clause);
    }
    static function update($clause) {
        $db = BoincDb::get();
        return $db->update_aux('credit_team', $clause);
    }
    static function get_list($where_clause, $order_clause, $limit) {
        $db = BoincDB::get();
        return $db->get_list('team', 'credit_team', 'id', 'teamid', 'BoincCreditTeam', '*', $where_clause, $order_clause, $limit);
    }
}

#[AllowDynamicProperties]
class BoincToken {

    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('token', 'BoincToken', $clause);
    }

    static function lookup_valid_token($userid, $token, $type) {
        $db = BoincDb::get();
        $token = BoincDb::escape_string($token);
        $type = BoincDb::escape_string($type);
        $now = time();
        return self::lookup("userid=$userid and token='$token' and expire_time > $now and type = '$type'");
    }

    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('token', 'BoincToken', $where_clause);
    }

    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('token', $clause);
    }

    static function get_list($where_clause, $order_clause, $limit) {
        $db = BoincDB::get();
        return $db->get_list('token', 'userid', 'type', 'create_time', 'expire_time', 'BoincToken', '*', $where_clause, $order_clause, $limit);
    }

    static function delete_token($where_clause) {
        $db = BoincDb::get();
        $db->delete_aux('token', $where_clause);
        return $db->affected_rows();
    }

    static function delete_expired() {
        $db = BoincDb::get();
        $now = time();
        $db->delete_aux('token', "expire_time < $now");
        return $db->affected_rows();
    }

    static function delete_for_user($user_id) {
        $db = BoincDb::get();
        $db->delete_aux('token', "userid=$user_id");
        return $db->affected_rows();
    }

}

#[AllowDynamicProperties]
class BoincUserDeleted {
    static function insert_user($user) {
        $now = time();
        $cpid = md5($user->cross_project_id.$user->email_addr);
        $clause = "(userid, public_cross_project_id, create_time) values ($user->id, '$cpid', $now)";
        $db = BoincDb::get();
        return $db->insert('user_deleted', $clause);
    }

    static function delete_expired() {
        $db = BoincDb::get();
        $expire_time = time() - 60*86400; //60 days ago
        $db->delete_aux('user_deleted', "create_time < $expire_time");
        return $db->affected_rows();
    }

}

#[AllowDynamicProperties]
class BoincHostDeleted {
    static function insert_hosts_for_user($user) {
        $now = time();
        $clause = "select id, host_cpid, $now from host where userid = $user->id";
        $db = BoincDb::get();
        return $db->insert('host_deleted', $clause);
    }

    static function delete_expired() {
        $db = BoincDb::get();
        $expire_time = time() - 60*86400; //60 days ago
        $db->delete_aux('host_deleted', "create_time < $expire_time");
        return $db->affected_rows();
    }

}

#[AllowDynamicProperties]
class BoincConsent {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('consent', 'BoincConsent', $clause);
    }

    static function enum($where_clause) {
        $db = BoincDb::get();
        return $db->enum('consent', 'BoincConsent', $where_clause);
    }

    static function insert ($clause) {
        $db = BoincDb::get();
        return $db->insert('consent', $clause);
    }

    static function update ($clause) {
        $db = BoincDb::get();
        return $db->update_aux('consent', $clause);
    }

    static function delete($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('consent', $clause);
    }

    static function delete_for_user($user_id) {
        $db = BoincDb::get();
        $db->delete_aux('consent', "userid=$user_id");
        return $db->affected_rows();
    }

}

#[AllowDynamicProperties]
class BoincConsentType {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('consent_type', 'BoincConsentType', $clause);
    }

    static function enum($where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('consent_type', 'BoincConsentType', $where_clause, $order_clause);
    }

    static function insert ($clause) {
        $db = BoincDb::get();
        return $db->insert('consent_type', $clause);
    }

    static function update ($clause) {
        $db = BoincDb::get();
        return $db->update_aux('consent_type', $clause);
    }

    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'consent_type');
    }

    function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('consent_type', $clause);
    }

}

// Class to interface with SQL View latest_consent. Only read
// operations permitted.
#[AllowDynamicProperties]
class BoincLatestConsent {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('latest_consent', 'BoincLatestConsent', $clause);
    }

    static function enum($where_clause, $order_clause=null) {
        $db = BoincDb::get();
        return $db->enum('latest_consent', 'BoincLatestConsent', $where_clause, $order_clause);
    }
}

// DEPRECATED: use BoincDb::escape_string where possible
//
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

?>
