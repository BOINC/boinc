<?php

require_once("../inc/db_conn.inc");
require_once("../inc/util.inc");

define('BOLT_MODE_LESSON', 1);
define('BOLT_MODE_SHOW', 2);
define('BOLT_MODE_SCORE', 3);
define('BOLT_MODE_ANSWER', 4);
define('BOLT_MODE_FINISHED', 5);

define('BOLT_ACTION_NONE', 0);
define('BOLT_ACTION_NEXT', 1);
define('BOLT_ACTION_PREV', 2);
define('BOLT_ACTION_SUBMIT', 3);
define('BOLT_ACTION_QUESTION', 4);

class BoltDb extends DbConn {
    static $instance;

    static function get() {
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
            $user = parse_config($config, '<bolt_db_user>');
            $passwd = parse_config($config, '<bolt_db_passwd>');
            $host = parse_config($config, '<bolt_db_host>');
            $name = parse_config($config, '<bolt_db_name>');
            if ($host == null) {
                $host = "localhost";
            }
            $instance = new DbConn();
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

class BoltUser {
    static $cache;
    static function lookup_userid($id) {
        $db = BoincDb::get();
        return $db->lookup('bolt_user', 'BoltUser', "user_id=$id");
    }
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_user', $clause);
    }
    static function lookup(&$user) {
        if (!$user) return;
        if (isset($user->bolt)) return;
        if (isset(self::$cache[$user->id])) {
            $bolt = self::$cache[$user->id];
        } else {
            $bolt = self::lookup_userid($user->id);
            if (!$bolt) {
                self::insert("(user_id) values ($user->id)");
                $bolt = self::lookup_userid($user->id);
            }
            self::$cache[$user->id] = $bolt;
        }
        $user->bolt = $bolt;
    }
    function update($clause) {
        $db = BoltDb::get();
        $clause = "$clause where user_id=$this->user_id";
        return $db->update_aux('bolt_user', $clause);
    }
}

class BoltCourse {
    static function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_course', $clause);
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_course', 'BoltCourse');
    }
    static function enum() {
        $db = BoltDb::get();
        return $db->enum('bolt_course', 'BoltCourse');
    }
}

class BoltEnrollment {
    function insert($clause) {
        $db = BoltDb::get();
        return $db->insert('bolt_enrollment', $clause);
    }
    function lookup($user_id, $course_id) {
        $db = BoltDb::get();
        return $db->lookup('bolt_enrollment', 'BoltEnrollment', "user_id=$user_id and course_id=$course_id");
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update_aux('bolt_enrollment', "$clause where user_id=$this->user_id and course_id=$this->course_id");
    }
    function delete($user_id, $course_id) {
        $db = BoltDb::get();
        $db->delete_aux('bolt_enrollment', "user_id=$user_id and course_id=$course_id");
    }
}

class BoltView {
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_view', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_view', 'BoltView');
    }
    function update($clause) {
        $db = BoltDb::get();
        $db->update($this, 'bolt_view', $clause);
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_view', 'BoltView', $clause);
    }
}

class BoltResult {
    static function insert($clause) {
        $db = BoltDb::get();
        $ret = $db->insert('bolt_result', $clause);
        if (!$ret) return null;
        return $db->insert_id();
    }
    static function lookup_id($id) {
        $db = BoltDb::get();
        return $db->lookup_id($id, 'bolt_result', 'BoltResult');
    }
    static function enum($clause) {
        $db = BoltDb::get();
        return $db->enum('bolt_view', 'BoltResult', $clause);
    }
}

?>
