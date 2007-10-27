<?php

require_once("../inc/db_conn.inc");

class BoincDb extends DbConn {
    public static $instance;

    static function get() {
        if (!isset($instance)) {
            $instance = new DbConn();
            $retval = $instance->init_conn(
                "<db_user>", "<db_passwd>", "<db_host>", "<db_name>"
            );
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
    static function lookup_auth($auth) {
        $db = BoincDb::get();
        return $db->lookup('user', 'BoincUser', "authenticator='$auth'");
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
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('user', 'BoincUser', $clause);
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
    static function enum($clause) {
        $db = BoincDb::get();
        return $db->enum('team', 'BoincTeam', $clause);
    }
}

class BoincTeamDelta {
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
    function delete() {
        return $db->delete($this, 'host');
    }
}

class BoincResult {
    static function count($clause) {
        $db = BoincDb::get();
        return $db->count('result', $clause);
    }
}

?>
