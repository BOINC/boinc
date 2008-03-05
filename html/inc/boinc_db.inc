<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

class BoincDb extends DbConn {
    static $instance;

    static function get($readonly = false) {
        global $generating_xml;
        if (web_stopped()) {
            if ($generating_xml) {
                xml_error(-183);
            } else {
                page_head("Page not available");
                echo "This page requires database access.
                    Our database server is temporarily shut down for maintenance.
                    Please try again later.
                ";
                page_tail();
            }
            exit();
        }
        if (!isset($instance)) {
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
                $retval = $instance->init_conn($user, $passwd, $replica_host, $name);
                if ($retval) return $instance;
            }
            $retval = $instance->init_conn($user, $passwd, $host, $name);
            if (!$retval) return null;
        }
        return $instance;
    }
    static function escape_string($string) {
        $db = self::get();
        return $db->base_escape_string($string);
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
        return $db->insert('user', $clause);
    }
    function delete() {
        $db = BoincDb::get();
        return $db->delete($this, 'user');
    }
}

class BoincTeam {
    static $cache;
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('team', $clause);
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
    static function update_aux($clause) {
        $db = BoincDb::get();
        return $db->update_aux('result', $clause);
    }
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'result', 'BoincResult');
    }
}

class BoincWorkunit {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'workunit', 'BoincWorkunit');
    }
    static function insert($clause) {
        $db = BoincDb::get();
        $ret = $db->insert('workunit', $clause);
        if (!$ret) return $ret;
        return $db->insert_id();
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
}

?>
