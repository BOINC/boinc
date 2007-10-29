<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

class BoincDb extends DbConn {
    static $instance;

    static function get($readonly = false) {
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
}

class BoincUser {
    static function lookup($clause) {
        $db = BoincDb::get();
        return $db->lookup('user', 'BoincUser', $clause);
    }

    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'user', 'BoincUser');
    }

    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('user', $clause);
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'user', $clause);
    }
    static function enum($clause, $clause2=null) {
        $db = BoincDb::get();
        return $db->enum('user', 'BoincUser', $clause, $clause2);
    }
    static function insert($clause) {
        $db = BoincDb::get();
        return $db->insert('user', $clause);
    }
}

class BoincTeam {
    static function lookup_id($id) {
        $db = BoincDb::get();
        return $db->lookup_id($id, 'team', 'BoincTeam');
    }
    function update($clause) {
        $db = BoincDb::get();
        return $db->update($this, 'team', $clause);
    }
    static function enum($clause, $clause2=null) {
        $db = BoincDb::get();
        return $db->enum('team', 'BoincTeam', $clause, $clause2);
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
}

class BoincPost {
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('post', $clause);
    }
}

class BoincProfile {
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
}

?>
