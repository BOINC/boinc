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

$d = dirname(__FILE__);
require_once("$d/db_conn.inc");
require_once("$d/util_basic.inc");

class BoincDb extends DbConn {
    static $instance;

    // connect to the database (possibly to a read-only replica)
    // NOTE: choice of replica can be made only at the page level.
    // If there's a page that's guaranteed to do only reads, put
    // BoincDb::get(true);
    // at the top of it.
    //
    static function get_aux($readonly) {
        $config = get_config();
        $user = parse_config($config, '<db_user>');
        $passwd = parse_config($config, '<db_passwd>');
        $host = parse_config($config, '<db_host>');
        $replica_host = parse_config($config, '<replica_db_host>');
        $name = parse_config($config, '<db_name>');
        if ($host == null) {
            $host = "localhost";
        }
        $instance = new DbConn();
        if ($readonly && $replica_host) {
            $x = parse_config($config, '<replica_db_user>');
            if ($x) $user = $x;
            $x = parse_config($config, '<replica_db_passwd>');
            if ($x) $passwd = $x;
            $x = parse_config($config, '<replica_db_name>');
            if ($x) $name = $x;
            $retval = $instance->init_conn($user, $passwd, $replica_host, $name);
            if ($retval) {
                self::$instance = $instance;
                return $instance;
            }
        }
        $retval = $instance->init_conn($user, $passwd, $host, $name);
        if (!$retval) {
            $instance = null;
        } else {
            $instance->do_query("use $name");
                // needed for places where we do direct queries
        }
        self::$instance = $instance;
        return $instance;
    }

    // same, but
    // 1) check for a cached connection
    // 2) check whether the "stop_web" trigger file is present
    //
    static function get($readonly = false) {
        if (!isset(self::$instance)) {
            if (web_stopped()) {
                show_page("Project down for maintenance",
                    "Please check back in a few hours."
                );
                exit;
            }
            self::get_aux($readonly);
            if (!self::$instance) {
                show_page(
                    "Project is down",
                    "The project's database server is down.
                    Please check back in a few hours."
                );
                exit;
            }
        }
        return self::$instance;
    }

    static function escape_string($string) {
        $db = self::get();
        return parent::base_escape_string(trim($string));
    }
    static function error() {
        $db = self::get();
        return $db->base_error();
    }
}

class BoincUser {
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
    function sum($field) {
        $db = BoincDb::get();
        return $db->sum('user', $field);
    }
}

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
    static function enum($clause, $clause2=null) {
        $db = BoincDb::get();
        return $db->enum('team', 'BoincTeam', $clause, $clause2);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('team', 'BoincTeam', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'team');
    }
}

class BoincTeamDelta {
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('team_delta', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('team_delta', 'BoincTeamDelta', $clause);
    }
}

class BoincHost {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'host', 'BoincHost');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'host', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'host');
    }
    static function enum($clause, $clause2=null) {
        $db = BoincDb::get();
        return $db->enum('host', 'BoincHost', $clause, $clause2);
    }
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('host', $clause);
    }
}

class BoincResult {
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('result', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('result', 'BoincResult', $clause);
    }
	static function enum_fields($fields, $where_clause, $order_clause) {
        $db = BoincDb::get();
		return $db->enum_fields('result', 'BoincResult', $fields, $where_clause, $order_clause);
	}
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'result', $clause);
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('result', $clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'result', 'BoincResult');
    }
    static function lookup_name($name) {
        $db = BoincDb::get();
        return $db->lookup('result', 'BoincResult', "name='$name'");
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'result');
    }
}

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
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('workunit', 'BoincWorkunit', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'workunit', $clause);
    }
}

class BoincApp {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'app', 'BoincApp');
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('app', 'BoincApp', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('app', 'BoincApp', $clause);
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
}

class BoincAppVersion {
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('app_version', 'BoincAppVersion', $clause);
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

class BoincProfile {
    static function lookup_fields($fields, $clause) {
        $db = BoincDb::get();
        return $db->lookup_fields('profile', 'BoincProfile', $fields, $clause);
    }
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('profile', 'BoincProfile', $clause);
    }
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('profile', $clause);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('profile', $clause);
    }
    static function enum($clause, $clause2=null) {
        $db = BoincDb::get();
        return $db->enum('profile', 'BoincProfile', $clause, $clause2);
    }
    function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('profile', $clause);
    }
}

class BoincTeamAdmin {
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('team_admin', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('team_admin', 'BoincTeamAdmin', $clause);
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

class BoincPrivateMessage {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'private_messages', 'BoincPrivateMessage');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'private_messages', $clause);
    }
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('private_messages', 'BoincPrivateMessage', $clause);
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
    function delete_aux($clause) {
        $db = BoincDb::get();
        return $db->delete_aux('private_messages', $clause);
    }
}

class BoincPlatform {
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('platform', 'BoincPlatform', $clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'platform', 'BoincPlatform');
    }
}

class BoincHostAppVersion {
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('host_app_version', 'BoincHostAppVersion', $clause);
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
            if ($av->plan_class == $av2->plan_class && $av->version_num > $av2->version_num) {
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

?>
